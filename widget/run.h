#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <egl_ui/egl_ui_owner.h>

#include <widget/context.h>
#include <widget/event_matching.h>


namespace widget
{
    namespace private_detail_run
    {
        constexpr bool initialization_was_successful(event_result_processor<>) noexcept
        {
            return true;
        }

        constexpr bool initialization_was_successful(event_result_processor<bool> result) noexcept
        {
            return result.result;
        }

        constexpr bool initialization_was_successful(event_result_processor<window_configation> result) noexcept
        {
            return badcfg != result.result;
        }

        template<class Widget>
        bool call_initialization_event(const widget::window& window, Widget& widget) noexcept
        {
            return initialization_was_successful(apply_event(widget, window));
        }

        template<class Widget>
        class processor
        {
        public:
            template<class... Args>
            explicit processor(widget::window& window, Args&&... args) noexcept
                : widget_{ std::forward<Args>(args)... }
                , common_context_{ window }
            {}

#ifdef D_OS_WINDOWS
            std::nullopt_t operator () (const ui::size_event& e) noexcept
            {
                const auto new_size = e.sizes();
                if (new_size.width() > 0_px && new_size.height() > 0_px) [[likely]]
                {
                    D_ASSERT(new_size.width() <= cref_window().viewport.width());
                    D_ASSERT(new_size.height() <= cref_window().viewport.height());
                    ref_window().content_sizes(new_size);
                    apply_ui_event(e);
                }

                return std::nullopt;
            }
#endif

#ifdef D_OS_ANDROID
            void operator () (const ui::content_rect_changed_event&) noexcept
            {
                const auto new_viewport = app_ui_viewport_request(cref_window());
                if (!new_viewport) [[unlikely]]
                {
                    ui_fatal_debug(cref_window(), "content rect error: ui error: {}, egl error: {}",
                        ui::error_code(), eglGetError());
                    return;
                }

                    if (new_viewport != cref_window().viewport)
                    {
                        ref_window().viewport = new_viewport;

                        if (!initialize_or_update_common_context())
                        {
                            ui_fatal_debug(cref_window(), "update draw context error: ui error: {}, egl error: {}",
                                ui::error_code(), eglGetError());
                            return;
                        }
                    }

                return;
            }
#endif 

            void operator () (ui::redraw_needed_event) noexcept
            {
                e_bit_or_eq(combined_event_result_, widget::event_result::redraw);
            }

            template<ui::event_style Style>
            std::nullopt_t operator () (const ui::specialized_event<Style>& e) noexcept
            {
                apply_ui_event(e);
                return std::nullopt;
            }

            ui::milliseconds operator () (ui::idle_event) noexcept
            {
#ifdef D_OS_WINDOWS
                using namespace std::chrono_literals;
                constexpr std::chrono::steady_clock::duration min_update_time{ 35ms };

                if (e_bit_check(combined_event_result_, event_result::redraw))
                {
                    const auto now = std::chrono::steady_clock::now();

                    if ((now - redraw_time_cache) < min_update_time)
                    {
                        return ui::milliseconds::zero();
                    }

                    e_bit_clear(combined_event_result_, event_result::redraw);
                    redraw_time_cache = now;
                    draw();
                }

#else
                if (e_bit_extract(combined_event_result_, event_result::redraw))
                {
                    draw();
                }

#endif

                return ui::infinite;
            }

            constexpr const window& cref_window() const noexcept
            {
                return common_context_.cref_window();
            }

            constexpr window& ref_window() noexcept
            {
                return common_context_.ref_window();
            }

            bool initialize() noexcept
            {
                return initialize_or_update_common_context()
                    && call_initialization_event(cref_window(), widget_);
            }

        private:
            template<class Event>
            void apply_ui_event(const Event& e) noexcept
            {
                combine_event_result(combined_event_result_, apply_event(common_context_, e));
                
                {
                    const event_common_context e_cc{ e, common_context_ };
                    combine_event_result(combined_event_result_, apply_event(widget_, e_cc));
                }
            }

            void draw() noexcept
            {
                const egl_painting_owner painting_owner{ cref_window() };
                gl::viewport(cref_window().viewport);
                gl::clear(colors::gl_dialog_color_f);
                D_UNUSED(apply_event(widget_, common_context_));
            }


            bool initialize_or_update_common_context() noexcept
            {
                return call_initialization_event(cref_window(), common_context_);
            }

        private:
            Widget widget_;
            common_context_t<Widget> common_context_{};
            D_ONLY_OS_WINDOWS(std::chrono::steady_clock::time_point redraw_time_cache{});
            event_result combined_event_result_{ event_result::redraw };
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

        return ui::run_event_loop(processor.cref_window(), processor);
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