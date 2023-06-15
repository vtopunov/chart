#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <egl_ui/event_loop.h>

#include <widget/event_result.h>
#include <widget/widget_initializer.h>
#include <widget/window_builder.h>


namespace widget
{
    namespace private_detail_run
    {
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

        template<class Widget>
        struct processor
        {
            window& widget_window;
            Widget widget;
            event_result combined_event_result{ event_result::redraw };

            template<class... Args>
            explicit processor(widget::window& window, Args&&... args) noexcept
                : widget_window{ window }
                , widget{ std::forward<Args>(args)... }
            {}

            std::nullopt_t operator () (const ui::size_event& e) noexcept
            {
                widget_window.user_sizes_cache = e.sizes();
                D_ASSERT(widget_window.user_sizes_cache.width() <= width(widget_window));
                D_ASSERT(widget_window.user_sizes_cache.height() <= height(widget_window));

                combined_event_result |= apply_event(widget, e);
                return std::nullopt;
            }

            template<ui::event_style Style>
            std::nullopt_t operator () (const ui::specialized_event<Style>& e) noexcept
            {
                combined_event_result |= apply_event(widget, e);
                return std::nullopt;
            }

            ui::milliseconds_t operator () (ui::idle_event) noexcept
            {
                if (event_result::redraw == (combined_event_result & event_result::redraw))
                {
                    combined_event_result &= ~event_result::redraw;

                    const egl_painting_owner painting_lock{ widget_window };
                    apply_draw(widget, widget_window);
                }

                return ui::infinite;
            }
        };


        template<class Fn, class... Args>
        using return_t = std::remove_cv_t<
            decltype(std::declval<std::add_lvalue_reference_t<Fn>>()(std::declval<std::add_lvalue_reference_t<Args...>>()))
        >;

        template<class T>
        std::enable_if_t<std::is_same_v<return_t<T, widget_initializer>, void>, bool>
            call_initialize(widget_initializer& ini, T& processor) noexcept
        {
            processor(ini);
            return true;
        }

        template<class T>
        std::enable_if_t<std::is_same_v<return_t<T, widget_initializer>, bool>, bool>
            call_initialize(widget_initializer& ini, T& processor) noexcept
        {
            return processor(ini);
        }

        constexpr bool call_initialize(widget_initializer&, no_overloaded) noexcept
        {
            return true;
        }

        template<class Widget>
        bool initialize(window& wnd, Widget& wgt) noexcept
        {
            widget_initializer ini{ wnd };
            return call_initialize(ini, wgt)
                && wgt.apply([&ini, &wnd] (auto&... widgets) noexcept
                {
                    return (call_initialize(ini, widgets) && ... && true);
                }) && configure(wnd, ini.cfg());
        }
    }

    template<class Widget, class... Args>
    int run(widget::window& widget_window, Args&&... args) noexcept
    {
        if (!widget_window)
        {
            e_debug
            (
                "window error: window error: {}, egl error: {}",
                ui::error_code(),
                eglGetError()
            );
            return EXIT_FAILURE;
        }

        private_detail_run::processor<Widget> processor{ widget_window, std::forward<Args>(args)... };

        if (!private_detail_run::initialize(widget_window, processor.widget))
        {
            e_debug
            (
                "widget's initialize error: window error: {}, egl error: {}",
                ui::error_code(),
                eglGetError()
            );
            return EXIT_FAILURE;
        }

        return ui::run_event_loop(widget_window, processor);
    }
}