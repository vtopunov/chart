#include <utility/shaders_library.h>
#include <utility/user_gesture.h>

#include <egl_ui/egl_ui_owner.h>

#include "pix8map_test_texture_generate.h"


namespace
{
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
                constexpr auto gl_numeric_max = [] () noexcept
                {
                    constexpr auto px_digits = numeric_digits_v<pxoff_t>;
                    return numeric_max_v<pxoff_t> >> (px_digits - std::min(numeric_digits_v<GLfloat>, px_digits));
                } ();

                D_ASSERT(ssizes.x() <= gl_numeric_max);
                D_ASSERT(ssizes.y() <= gl_numeric_max);
                return fill_to<point2d>(gl_numeric_max) - ssizes;
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
            return with_zoom(vtrunc_to_px(sizes * step_mul));
        }

        [[nodiscard]]
        constexpr bool operator == (const figure_area&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const figure_area&) const noexcept = default;
    };

    class main_processor
    {
    public:
        [[nodiscard]]
        bool initialize(os::module_handle_t app) noexcept
        {
            egl_ = create_egl_ui(app);
            if (!egl_)
            {
                return false;
            }

            return reinitialize();
        }

        [[nodiscard]]
        bool reinitialize() noexcept
        {
            texture_ = pix8map_test_texture_generate(egl_.viewport / 4u);
            if (!texture_)
            {
                return false;
            }

            if (!shaders_.initialize(egl_.viewport, texture_))
            {
                return false;
            }

            area_ = default_area(egl_.viewport);
            return true;
        }

#if defined(D_OS_WINDOWS)
        std::nullopt_t operator () (const ui::mouse_wheel& e) noexcept
        {
            if (const auto new_area = area_.with_zoom_increase(e.rot()); new_area != area_)
            {
                area_ = new_area;
                need_redraw_ = true;
            }

            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_double_click&) noexcept
        {
            area_ = default_area(egl_.viewport);
            need_redraw_ = true;
            return std::nullopt;
        }
#endif

        std::nullopt_t operator () (const ui::mouse_move_event& e) noexcept
        {
#if  defined(D_OS_WINDOWS)
            if (!e.keys().is_left())
            {
                clear_gesture_cache(gesture_cache_);
                return std::nullopt;
            }
#endif

            const auto gesture = new_motion_user_gesture(gesture_cache_, e);
            const auto has_move = gesture.has_move();
            const auto has_zoom = gesture.has_zoom();

            if(has_move || has_zoom)
            {
                figure_area new_area{ area_ };

                if (has_move)
                {
                    new_area = new_area.with_shift(gesture.move, egl_.viewport);
                }

                if (has_zoom)
                {
                    new_area = new_area.with_zoom(gesture.zoom);
                }

                if(new_area != area_)
                {
                    area_ = new_area;
                    need_redraw_ = true;
                }
            }

            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_up_event&) noexcept
        {
            clear_gesture_cache(gesture_cache_);
            return std::nullopt;
        }

#ifdef D_OS_ANDROID
        void operator () (ui::content_rect_changed_event)
        {
            const auto new_viewport = app_ui_viewport_request(egl_);
            if (!new_viewport)
            {
                ui_fatal_debug(egl_, "content rect error: ui error: {}, egl error: {}",
                    ui::error_code(), eglGetError());
                return;
            }

            if (new_viewport != egl_.viewport)
            {
                egl_.viewport = new_viewport;

                if (!reinitialize())
                {
                    ui_fatal_debug(egl_, "reinitialize viewport error: ui error: {}, egl error: {}",
                        ui::error_code(), eglGetError());
                    return;
                }
            }

            return;
        }
#endif

        void operator () (ui::redraw_needed_event) noexcept
        {
            need_redraw_ = true;
        }

        ui::milliseconds operator () (ui::idle_event) noexcept
        {
            if (need_redraw_)
            {
                need_redraw_ = false;
                draw();
            }

            return ui::infinite;
        }

        int run()
        {
            draw();
            return ui::run_event_loop(egl_, *this);
        }

    private:
        void draw() const noexcept
        {
            const egl_painting_owner painting_owner{ egl_ };
            gl::viewport(egl_.viewport);
            gl::clear(gl::colors::white_f);
            shaders_.draw(area_.geometry());
        }

        [[nodiscard]]
        static figure_area default_area(pxsize2d viewport) noexcept
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
            shaders_library<vert::positioned_texture, frag::gray_texture_mix_color> lib{};
        };

        egl_ui_owner egl_{};
        shaders_lib shaders_{};
        gl::texture2d texture_;
        gesture_vpoint2d_t gesture_cache_{ invalid_gesture_vpoint };
        figure_area area_{};
        bool need_redraw_{ true };
        };
    }

int app_main(os::module_handle_t app) noexcept
{
    main_processor processor;

    if (!processor.initialize(app))
    {
        e_debug("create window error: ui error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    return processor.run();
}




