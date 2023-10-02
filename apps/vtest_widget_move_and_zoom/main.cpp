#include <utility/shader_library.h>

#include <widget/run.h>
#include <widget/gesture_cache.h>

#include "pix8map_test_texture_generate.h"


namespace
{
    template<class T>
    constexpr auto gl_max_v = [] () noexcept
    {
        static_assert(std::is_integral_v<T>);
        constexpr auto px_digits = numeric_digits_v<T>;
        return numeric_max_v<T> >> (px_digits - std::min(numeric_digits_v<GLfloat>, px_digits));
    } ();

    constexpr auto gl_pxz_max = gl_max_v<pxoff_t>;


    template<template<class> class Vec, class T>
    [[nodiscard]] constexpr std::enable_if_t<
        std::is_base_of_v<vec2<T>, Vec<T>>, Vec<T>
    > vclamp(const Vec<T>& v, const Vec<T>& v0, const Vec<T>& v1) noexcept
    {
        return
        {
            std::clamp(v._0, v0._0, v1._0),
            std::clamp(v._1, v0._1, v1._1),
        };
    }


    struct figure_center_position
    {
        pxoff2d position{};

        [[nodiscard]]
        constexpr figure_center_position with_clamp(pxsize2d fig_sizes, pxsize2d viewport) const noexcept
        {
            const auto min_position = -narrow2d<pxoff2d>((fig_sizes + fill_vec2(1_px)) / 2_px);
            const auto max_position = narrow2d<pxoff2d>((fig_sizes + 2_px * viewport) / 2_px);

            return
            {
                .position
                {
                    std::clamp(position.x(), min_position.x(), max_position.x()),
                    std::clamp(position.y(), min_position.y(), max_position.y())
                }
            };
        }

        [[nodiscard]]
        constexpr figure_center_position with_shift(pxoff2d shift) const noexcept
        {
            return { .position{ position + shift } };
        }

        [[nodiscard]]
        constexpr pxoff2d left_top(pxsize2d fig_sizes) const noexcept
        {
            return (2 * position - narrow2d<pxoff2d>(fig_sizes)) / 2;
        }

        [[nodiscard]]
        constexpr bool operator == (const figure_center_position&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const figure_center_position&) const noexcept = default;
    };

    struct figure_area
    {
        figure_center_position position{};
        pxsize2d sizes{};

        [[nodiscard]]
        constexpr rectangle<px::pxoff_t> geometry() const noexcept
        {
            return
            {
                .position{ position.left_top(sizes) },
                .sizes{ sizes }
            };
        }

        [[nodiscard]]
        constexpr figure_area with_shift(pxoff2d shift, pxsize2d viewport) const noexcept
        {
            return
            {
                .position{ position.with_shift(shift).with_clamp(sizes, viewport) },
                .sizes{ sizes }
            };
        }

        [[nodiscard]]
        figure_area with_zoom(pxoff2d zoom, pxsize2d viewport) const noexcept
        {
            const auto apply_zoom = [] (double value0, double value, double zoom) noexcept
            {
                const auto n_zoom = value / value0;
                return narrow<pxside_t>(std::clamp
                (
                    trunc_to_pxz((n_zoom > 1.0) ? (value + zoom * n_zoom) : (value + zoom)),
                    1_pxz,
                    gl_pxz_max
                ));
            };

            return
            {
                .position{ position },
                .sizes
                {
                    apply_zoom(viewport.width(), sizes.width(), zoom.x()),
                    apply_zoom(viewport.height(), sizes.height(), zoom.y()),
                }
            };
        }

        [[nodiscard]]
        constexpr figure_area with_zoom(pxoff2d zoom) const noexcept
        {
            constexpr auto min_zoom = [] (pxoff2d ssizes) noexcept
            {
                D_ASSERT(ssizes.x() >= 1_pxz);
                D_ASSERT(ssizes.y() >= 1_pxz);
                return fill_to<point2d>(1_pxz) - ssizes;
            };

            constexpr auto max_zoom = [] (pxoff2d ssizes) noexcept
            {
                D_ASSERT(ssizes.x() <= gl_pxz_max);
                D_ASSERT(ssizes.y() <= gl_pxz_max);
                return fill_to<point2d>(gl_pxz_max) - ssizes;
            };

            const auto ssizes = narrow2d<pxoff2d>(sizes);

            return
            {
                .position{ position },
                .sizes{ narrow2d<pxsize2d>(ssizes + vclamp(zoom, min_zoom(ssizes), max_zoom(ssizes))) }
            };
        }

        [[nodiscard]]
        figure_area with_zoom_increase(double rot) const noexcept
        {
            constexpr double mul{ 0.05 };
            const auto sign = 1 - 2 * std::signbit(rot);
            const auto step_mul = sign * pow(mul, abs(rot));
            return with_zoom(vtrunc_to_pxz(sizes * step_mul));
        }

        [[nodiscard]]
        constexpr bool operator == (const figure_area&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const figure_area&) const noexcept = default;
    };


    class main_widget
    {
    public:
        bool operator () (const widget::window& w) noexcept
        {
            texture_ = pix8map_test_texture_generate(w.viewport / 4u);
            if (!texture_)
            {
                return false;
            }

            if (!shaders_.initialize(w.viewport, texture_))
            {
                return false;
            }

            area_ = default_area(w.viewport);
            return true;
        }

        using mouse_move_event_type = widget::mouse_move_event<
            widget::gesture_cache,
            viewport_size2d
        >;

#if defined(D_OS_WINDOWS)
        using mouse_double_click_event_type = widget::mouse_double_click_event<
            viewport_size2d
        >;

        widget::event_result operator () (const ui::mouse_wheel_event& e) noexcept
        {
            if (const auto new_area = area_.with_zoom_increase(e.rot()); new_area != area_)
            {
                area_ = new_area;
                return widget::event_result::redraw;
            }

            return widget::event_result::idle;
        }

        constexpr widget::event_result operator () (mouse_double_click_event_type e) noexcept
        {
            area_ = default_area(e.get<viewport_size2d>());
            return widget::event_result::redraw;
        }

#endif

        widget::event_result operator () (mouse_move_event_type e)
        {
            if (const auto& gesture = e.get<widget::gesture_cache>())
            {
                figure_area new_area{ area_ };

                {
                    const auto viewport = e.get<viewport_size2d>();

                    if (const auto move = gesture.move())
                    {
                        new_area = new_area.with_shift(move, viewport);
                    }

                    if (const auto zoom = gesture.zoom())
                    {
                        new_area = new_area.with_zoom(zoom, viewport);
                    }
                }

                if (new_area != area_)
                {
                    area_ = new_area;
                    return widget::event_result::redraw;
                }
            }

            return widget::event_result::idle;
        }

        void operator () (widget::redraw_event<>) const noexcept
        {
            shaders_.draw(area_.geometry());
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) const noexcept
        {
            return fn(widget::ex_context_v<mouse_move_event_type>);
        }

    private:
        [[nodiscard]]
        static constexpr figure_area default_area(pxsize2d viewport) noexcept
        {
            const auto sizes = viewport / 2u;
            return
            {
                .position{ narrow2d<pxoff2d>(sizes) },
                .sizes{ sizes }
            };
        }

    private:
        class shaders_lib
        {
        public:
            bool initialize(pxsize2d viewport, gl::texture2d_resource texture) noexcept
            {
                const auto was_successful
                    = lib || lib.build();

                if (was_successful)
                {
                    lib.use();
                    lib.frag.s_texture.store(texture);
                    lib.frag.u_color.store(1.0f, 0.5f, 0.5f, 1.0f);
                    lib.vert.u_viewport.store(viewport);
                }

                return was_successful;
            }

            template<class T>
            void draw(const rectangle<T>& rc) const noexcept
            {
                lib.use();
                lib.vert.u_position.store(rc.position);
                lib.vert.u_size.store(rc.sizes);
                lib.vert.a_frame.draw();
            }

        private:
            shader_library<vert::positioned_texture, frag::gray_texture_mix_color> lib{};
        };

    private:
        shaders_lib shaders_{};
        gl::texture2d texture_{};
        figure_area area_{};
    };
}

int app_main(os::module_handle_t app) noexcept
{
    return widget::run<main_widget>(app);
}




