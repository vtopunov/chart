#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <egl_ui/run.h>

#include <widget/event_result.h>
#include <widget/window_configuration.h>
#include <widget/window.h>


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
        struct processor : window
        {
            static constexpr auto dialog_color = 0xf0f0f0_glrgb;

            Widget widget;
            event_result combined_event_result{ event_result::redraw };

            template<class... Args>
            explicit processor(os::module_handle_t app, Args&&... args) noexcept
                : window
                { 
                    .egl
                    {
                        egl_window_builder{}
                        .module(app)
                        .background(dialog_color)
                        .build()
                    },
                    .shaders{},
                    .temp_buffer{},
                    .user_sizes_cache{}
                }
                , widget{ std::forward<Args>(args)... }
            {}

            std::nullopt_t operator () (const ui::size_event& e) noexcept
            {
                user_sizes_cache = e.sizes();
                D_ASSERT(user_sizes_cache.width() <= width(*this));
                D_ASSERT(user_sizes_cache.height() <= height(*this));

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

                    const egl_painting_owner painting_lock{ window::egl };
                    apply_draw(widget, *this);
                }

                return ui::infinite;
            }
        };


        template<class Fn, class... Args>
        using return_t = std::remove_cv_t<
            decltype(std::declval<std::add_lvalue_reference_t<Fn>>()(std::declval<std::add_lvalue_reference_t<Args...>>()))
        >;

        template<class T>
        std::enable_if_t<std::is_same_v<return_t<T, window_configuration>, void>, bool> 
            call_window_configuration(window_configuration& cfg, T& processor) noexcept
        {
            processor(cfg);
            return true;
        }

        template<class T>
        std::enable_if_t<std::is_same_v<return_t<T, window_configuration>, bool>, bool>
            call_window_configuration(window_configuration& cfg, T& processor) noexcept
        {
            return processor(cfg);
        }

        constexpr bool call_window_configuration(window_configuration&, no_overloaded) noexcept
        {
            return true;
        }

        template<class Widget>
        bool configure(window& wnd, Widget& wgt) noexcept
        {
            window_configuration cfg{ wnd };
            return call_window_configuration(cfg, wgt) 
                && wgt.apply([&cfg, &wnd] (auto&... widgets) noexcept
                {
                    return (call_window_configuration(cfg, widgets) && ... && true);
                }) && cfg.configure();
        }
    }

    template<class Widget, class... Args>
    int run(os::module_handle_t app, Args&&... args) noexcept
    {
        private_detail_run::processor<Widget> processor{ app, std::forward<Args>(args)... };
        if (!processor.egl)
        {
            e_debug
            (
                "create window error: window error: {}, egl error: {}",
                 ui::error_code(), 
                eglGetError()
            );
            return EXIT_FAILURE;
        }

        if (!private_detail_run::configure(processor, processor.widget))
        {
            e_debug
            (
                "configure window error: window error: {}, egl error: {}",
                ui::error_code(),
                eglGetError()
            );
            return EXIT_FAILURE;
        }

        return egl_ui::run(processor.egl, processor);
    }
}