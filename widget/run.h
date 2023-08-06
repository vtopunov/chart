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
        constexpr void combine(event_result& combined_result, const event_result new_result) noexcept
        {
            e_bit_or_eq(combined_result, new_result);
        }

        constexpr void combine(event_result&, std::nullopt_t) noexcept
        {}

        template<class Widget, class Event>
        void apply_event(event_result& combined_result, Widget& widget, const Event& e) noexcept
        {
            combine(combined_result, call_event(widget, e));

            widget.apply([&combined_result, &e] (auto&... widgets) noexcept
            {
                (combine(combined_result, call_event(widgets, e)), ...);
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

        struct processor_construct_t
        {};

        constexpr processor_construct_t processor_construct{};

        template<class Widget>
        struct processor
        {
            window& widget_window;
            Widget widget;
            event_result combined_event_result{ event_result::redraw };

            template<class... Args>
            explicit processor(processor_construct_t, widget::window& window, Args&&... args) noexcept
                : widget_window{ window }
                , widget{ std::forward<Args>(args)... }
            {}

#ifdef D_OS_WINDOWS
            std::nullopt_t operator () (const ui::size_event& e) noexcept
            {
                const auto new_size = e.sizes();
                if (new_size.width() > 0_px && new_size.height() > 0_px) [[likely]]
                {
                    widget_window.user_sizes_cache = new_size;
                    D_ASSERT(widget_window.user_sizes_cache.width() <= width(widget_window));
                    D_ASSERT(widget_window.user_sizes_cache.height() <= height(widget_window));
                    apply_event(combined_event_result, widget, e);
                }

                return std::nullopt;
            }
#endif

            template<ui::event_style Style>
            std::nullopt_t operator () (const ui::specialized_event<Style>& e) noexcept
            {
                apply_event(combined_event_result, widget, e);
                return std::nullopt;
            }

            ui::milliseconds_t operator () (ui::idle_event) noexcept
            {
#ifdef D_OS_WINDOWS
                using namespace std::chrono_literals;
                constexpr std::chrono::steady_clock::duration min_update_time{ 35ms };

                if (e_bit_check(combined_event_result, event_result::redraw))
                {
                    const auto now = std::chrono::steady_clock::now(); 

                    if ((now - widget_window.redraw_time_cache) < min_update_time)
                    {
                        return ui::milliseconds_t::zero();
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
                const egl_painting_owner painting_lock{ widget_window };
                apply_draw(widget, widget_window);
            }
        };


        template<class Fn, class Arg>
        using return_t = std::remove_cv_t<
            decltype(std::declval<std::add_lvalue_reference_t<Fn>>()(std::declval<std::add_lvalue_reference_t<Arg>>()))
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

            return wnd
                && call_initialize(ini, wgt)
                && wgt.apply([&ini] (auto&... widgets) noexcept
            {
                return (call_initialize(ini, widgets) && ... && true);
            }) && configure(wnd, ini.cfg());
        }
    }

    template<class Widget, class... Args>
    int run(widget::window& widget_window, Args&&... args) noexcept
    {
        private_detail_run::processor<Widget> processor
        {
            private_detail_run::processor_construct,
            widget_window,
            std::forward<Args>(args)...
        };

        if (!private_detail_run::initialize(widget_window, processor.widget)) [[unlikely]]
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

    template<class Widget, class... Args>
    int run(os::module_handle_t app, Args&&... args) noexcept
    {
        auto window = widget::window_builder{}
            .module(app)
            .build();

        return run<Widget>(window, std::forward<Args>(args)...);
    }
}