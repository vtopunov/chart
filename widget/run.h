#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <egl_ui/egl_ui_owner.h>

#include <widget/event.h>
#include <widget/window_configation.h>


namespace widget
{
    namespace private_detail_run
    {
        template<class Widget, class Event>
        void apply_event(event_result_type_t<Event>& result, Widget& widget, const Event& e) noexcept
        {
            call_widget_event(result, widget, e);

            widget.apply([&result, &e] (auto&... widgets) noexcept
            {
                (call_widget_event(result, widgets, e), ...);
            });
        }

        struct processor_construct_t
        {};

        constexpr processor_construct_t processor_construct{};

        template<class Widget>
        struct processor
        {
            Widget widget;
            widget::window& widget_window;
            D_ONLY_OS_ANDROID(window_configation cfg{ nocfg });
            event_result combined_event_result{ event_result::redraw };

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
                    widget_window.content_sizes_cache = new_size;
                    D_ASSERT(widget_window.content_sizes_cache.width() <= widget_window.viewport.width());
                    D_ASSERT(widget_window.content_sizes_cache.height() <= widget_window.viewport.height());
                    apply_event(combined_event_result, widget, e);
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

                        if (!configure(widget_window, cfg)) [[unlikely]]
                        {
                            ui_fatal_debug(widget_window, "reinitialize viewport error: ui error: {}, egl error: {}",
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
                apply_event(combined_event_result, widget, e);
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

                    if ((now - widget_window.redraw_time_cache) < min_update_time)
                    {
                        return ui::milliseconds::zero();
                    }

                    e_bit_clear(combined_event_result, event_result::redraw);
                    widget_window.redraw_time_cache = now;
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
                apply_event(combined_event_result, widget, static_cast<const redraw_event&>(widget_window));
            }

            bool initialize() noexcept
            {
#ifndef D_OS_ANDROID
                window_configation cfg{ nocfg };
#endif

                apply_event(cfg, widget, static_cast<const init_event&>(widget_window));
                return configure(widget_window, cfg);
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