#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <egl_ui/run.h>

#include <widget/window.h>

namespace widget
{
    namespace private_detail_run
    {
        constexpr auto dialog_color = 0xf0f0f0_glrgb;

        template<class Widget>
        bool apply_initialize(Widget& widget, window& egl) noexcept;

        template<class Widget, class Event>
        decltype(auto) apply_event(Widget& widget, const Event& e) noexcept;

        template<class Widget>
        void apply_draw(Widget& widget, const window& window) noexcept;

        template<class Widget>
        struct processor
        {
            Widget widget;
            window window;
            event_result combined_event_result;

            std::nullopt_t operator () (const ui::mouse_down_event& e) noexcept
            {
                combined_event_result |= apply_event(widget, e);
                return std::nullopt;
            }

            std::nullopt_t operator () (const ui::mouse_up_event& e) noexcept
            {
                combined_event_result |= apply_event(widget, e);
                return std::nullopt;
            }

            std::nullopt_t operator () (const ui::mouse_move_event& e) noexcept
            {
                combined_event_result |= apply_event(widget, e);
                return std::nullopt;
            }

            ui::milliseconds_t operator () (ui::idle_event) noexcept
            {
                if (event_result::redraw == (combined_event_result & event_result::redraw))
                {
                    combined_event_result &= ~event_result::redraw;

                    const egl_painting_owner painting_lock{ window.egl };
                    gl::clear(dialog_color);
                    apply_draw(widget, window);
                }

                return ui::infinite;
            }
        };

        template<class Widget>
        bool apply_initialize(Widget& widget, window& w) noexcept
        {
            return widget.initialize(w)
                && widget.apply([&w] (auto&... widgets) noexcept
            {
                return (widgets.initialize(w) && ... && true);
            });
        }

        template<class Widget, class Event>
        decltype(auto) apply_event(Widget& widget, const Event& e) noexcept
        {
            return widget.apply([&e] (auto&... widgets) noexcept
            {
                event_result combined_result{ event_result::idle };
                ((combined_result |= call_event(widgets, e)), ...);
                return combined_result;
            });
        }

        template<class Widget>
        void apply_draw(Widget& widget, const window& w) noexcept
        {
            widget.apply([&w] (auto&... widgets) noexcept
            {
                (widgets.draw(w), ...);
            });
        }
    }

    template<class Widget, class... Args>
    int run(os::module_handle_t app, Args&&... args) noexcept
    {
        private_detail_run::processor<Widget> processor
        {
            .widget{ std::forward<Args>(args)... },
            .window
            {
                .egl{ create_egl_window(app) },
                .shaders{},
                .temp_buffer{}
            },
            .combined_event_result{ event_result::redraw }
        };

        if (!processor.window.egl)
        {
            e_debug("create window error: window error: {}, egl error: {}",
                    ui::error_code(), eglGetError());
            return EXIT_FAILURE;
        }

        if (!private_detail_run::apply_initialize(processor.widget, processor.window))
        {
            return EXIT_FAILURE;
        }

        return egl_ui::run(processor.window.egl, processor);
    }
}