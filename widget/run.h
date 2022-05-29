#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <egl/event_loop.h>

#include <widget/event_context.h>

namespace widget
{
    namespace private_detail_widgets_run
    {
        template<class Group>
        class widgets_wrapper
        {
        public:
            constexpr widgets_wrapper(Group& group) noexcept
                : widgets_ref_{ group }
            {}

            template<class Event>
            std::nullopt_t apply_event(const Event& e) noexcept
            {
                widget::event_context context{ .need_redraw{ false } };

                widgets_ref_.apply
                (
                    [&context, &e](auto&... widget) noexcept
                    {
                        (..., widget(context, e));
                    }
                );

                if (context.need_redraw)
                {
                    draw();
                }

                return std::nullopt;
            }


            std::nullopt_t operator () (const ui::mouse_lbutton_down_event& e) noexcept
            {
                return apply_event(e);
            }

            std::nullopt_t operator () (const ui::mouse_lbutton_up_event& e) noexcept
            {
                return apply_event(e);
            }

            std::nullopt_t operator () (const ui::mouse_move_event& e) noexcept
            {
                return apply_event(e);
            }

            void draw() noexcept
            {
                if (const auto lock = egl::begin_painting(egl_))
                {
                    constexpr auto dialog_color = 0xf0f0f0_glrgb;

                    gl::clear(dialog_color);

                    widgets_ref_.apply
                    (
                        [this](auto&... widget) noexcept
                        {
                            (..., widget.draw(this->temp_buffer_));
                        }
                    );
                }
            }

            bool initialize() noexcept
            {
                return widgets_ref_.apply([viewport_sizes = egl::sizes(egl_)](auto&... widgets) noexcept
                {
                    return (... && widgets.initialize(viewport_sizes));
                });
            }

            int run() noexcept
            {
                if (!egl_)
                {
                    egl_ = egl::window_builder{}.build();
                }

                if (!egl_)
                {
                    e_debug("create window error: window error: {}, egl error: {}",
                        ui::error_code(), eglGetError());
                    return EXIT_FAILURE;
                }

                if (!initialize())
                {
                    return EXIT_FAILURE;
                }

                draw();

                return egl::run(egl_, *this);
            }

            egl::window egl_{};
            buffer_t temp_buffer_{};
            Group& widgets_ref_;
        };

        template<class... Widgets>
        struct tuple : std::tuple<Widgets...>
        {
            using base_type = std::tuple<Widgets...>;
            using base_type::tuple;

            template<class Callable>
            decltype(auto) apply(Callable&& fn) noexcept
            {
                return std::apply(std::forward<Callable>(fn), static_cast<base_type&>(*this));
            }
        };
    }

    template<class... Widgets>
    int run(Widgets&&... widgets) noexcept
    {
        using tuple_t = private_detail_widgets_run::tuple<Widgets...>;

        tuple_t t{ std::forward<Widgets>(widgets)... };

        return private_detail_widgets_run::widgets_wrapper<tuple_t>{ t }.run();
    }

    template<class Widget>
    int run() noexcept
    {
        Widget w{};
        return private_detail_widgets_run::widgets_wrapper<Widget>{ w }.run();
    }
}