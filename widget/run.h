#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <egl_ui/egl_ui_owner.h>

#include <widget/draw_context.h>
#include <widget/event_matching.h>


namespace widget
{
    namespace private_detail_run
    {
        struct processor_construct_t
        {};

        constexpr processor_construct_t processor_construct{};

        template<class Widget>
        bool initialize(const widget::window& window, Widget& widget) noexcept
        {
            window_configation cfg{ nocfg };
            apply_event(cfg, widget, window);
            return badcfg != cfg;
        }

        template<class Widget>
        struct processor
        {
            Widget widget;
            widget_common_draw_context_t<Widget> common_draw_context{};
            D_ONLY_OS_WINDOWS(std::chrono::steady_clock::time_point redraw_time_cache{});
            widget::window& widget_window;
            event_result combined_event_result{ event_result::redraw };

            template<class Fn>
            decltype(auto) apply(Fn fn) noexcept
            {
                return fn(common_draw_context, widget);
            }

            template<class... Args>
            explicit processor(processor_construct_t, widget::window& window, Args&&... args) noexcept
                : widget{ std::forward<Args>(args)... }
                , widget_window{ window }
            {}

#ifdef D_OS_WINDOWS
            std::nullopt_t operator () (const ui::size_event& e) noexcept
            {
                const auto new_size = e.sizes();
                if (new_size.width() > 0_px && new_size.height() > 0_px) [[likely]]
                {
                    D_ASSERT(new_size.width() <= widget_window.viewport.width());
                    D_ASSERT(new_size.height() <= widget_window.viewport.height());
                    apply_event(combined_event_result, *this, e);
                }

                return std::nullopt;
            }
#endif

#ifdef D_OS_ANDROID
            void operator () (const ui::content_rect_changed_event&) noexcept
            {
                const auto new_viewport = app_ui_viewport_request(widget_window);
                if (!new_viewport) [[unlikely]]
                {
                    ui_fatal_debug(widget_window, "content rect error: ui error: {}, egl error: {}",
                        ui::error_code(), eglGetError());
                    return;
                }

                    if (new_viewport != widget_window.viewport)
                    {
                        widget_window.viewport = new_viewport;

                        if (!initialize_or_update_common_draw_context())
                        {
                            ui_fatal_debug(widget_window, "update draw context error: ui error: {}, egl error: {}",
                                ui::error_code(), eglGetError());
                            return;
                        }
                    }

                return;
            }
#endif 

            void operator () (ui::redraw_needed_event) noexcept
            {
                combined_event_result |= widget::event_result::redraw;
            }

            template<ui::event_style Style>
            std::nullopt_t operator () (const ui::specialized_event<Style>& e) noexcept
            {
                apply_event(combined_event_result, *this, e);
                return std::nullopt;
            }

            ui::milliseconds operator () (ui::idle_event) noexcept
            {
#ifdef D_OS_WINDOWS
                using namespace std::chrono_literals;
                constexpr std::chrono::steady_clock::duration min_update_time{ 35ms };

                if (e_bit_check(combined_event_result, event_result::redraw))
                {
                    const auto now = std::chrono::steady_clock::now();

                    if ((now - redraw_time_cache) < min_update_time)
                    {
                        return ui::milliseconds::zero();
                    }

                    e_bit_clear(combined_event_result, event_result::redraw);
                    redraw_time_cache = now;
                    draw();
                }

#else
                if (e_bit_extract(combined_event_result, event_result::redraw))
                {
                    draw();
                }

#endif

                return ui::infinite;
            }

            void draw() noexcept
            {
                const egl_painting_owner painting_owner{ widget_window };
                gl::viewport(widget_window.viewport);
                gl::clear(colors::gl_dialog_color_f);
                apply_event(combined_event_result, widget, common_draw_context);
            }

            bool initialize() noexcept
            {
                return initialize_or_update_common_draw_context()
                    && private_detail_run::initialize(widget_window, widget);
            }

            bool initialize_or_update_common_draw_context() noexcept
            {
                return private_detail_run::initialize(widget_window, common_draw_context);
            }
        };
    }

    template<class Widget, class... Args>
    int run(widget::window& window, Args&&... args) noexcept
    {
        if (!window) [[unlikely]]
        {
            e_debug
            (
                "create window error: window error: {}, egl error: {}",
                ui::error_code(),
                eglGetError()
            );
            return EXIT_FAILURE;
        }

        private_detail_run::processor<Widget> processor
        {
            private_detail_run::processor_construct,
            window,
            std::forward<Args>(args)...
        };

        if (!processor.initialize()) [[unlikely]]
        {
            e_debug
            (
                "widget's initialize error: window error: {}, egl error: {}",
                ui::error_code(),
                eglGetError()
            );
            return EXIT_FAILURE;
        }

        return ui::run_event_loop(processor.widget_window, processor);
    }

    template<class Widget, class... Args>
    int run(os::module_handle_t app, Args&&... args) noexcept
    {
        auto window = widget::window_builder{}
            .module(app)
            .build();

        return run<Widget>(window, std::forward<Args>(args)...);
    }
}