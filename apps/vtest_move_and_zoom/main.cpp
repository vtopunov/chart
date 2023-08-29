#include <random>
#include <variant>

#include <core/round.h>
#include <core/static_vector.h>

#include <debug/debug.h>

#include <utility/shaders_library.h>
#include <egl_ui/egl_ui_owner.h>


namespace
{             
    template<template<class> class Vec, class T>
    [[nodiscard]] constexpr std::enable_if_t<
        std::is_base_of_v<vec2<T>, Vec<T>>, Vec<T>
    > vabs(const Vec<T>& v) noexcept
    {
        return { constexpr_abs(v._0), constexpr_abs(v._1) };
    }

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

    template<class T> [[nodiscard]]
    pxoff2d vtrunc_to_px(const vec2<T>& p) noexcept
    {
        return
        {
            trunc_cast<pxoff_t>(p._0),
            trunc_cast<pxoff_t>(p._1)
        };
    }

    template<class L, class R>
    [[nodiscard]] constexpr decltype(auto) sqr_distance(const point2d<L>& p0, const point2d<R>& p1) noexcept
    {
        const auto dpt = as_signed(p0 - p1);
        return to_unsigned_or(dpt.x() * dpt.x()) + to_unsigned_or(dpt.y() * dpt.y());
    };

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
                .sizes{ narrow2d<pxsize2d>(ssizes + vclamp(zoom, min_zoom(ssizes), max_zoom(ssizes))  ) }
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


    struct gesture
    {
        pxoff2d move{ 0_pxz, 0_pxz };
        pxoff2d zoom{ 0_pxz, 0_pxz };

        template<class M, class Z> 
        [[nodiscard]] static gesture instance(const point2d<M>& move, const point2d<Z>& zoom) noexcept
        {
            return 
            { 
                .move{ vtrunc_to_px(move) }, 
                .zoom{ vtrunc_to_px(zoom) }
            };
        }

        template<class T> 
        [[nodiscard]] static gesture instance(const point2d<T>& move) noexcept
        {
            return { .move{ vtrunc_to_px(move) } };
        }

        [[nodiscard]]
        constexpr bool has_move() const noexcept
        {
            return has_offset(move);
        }

        [[nodiscard]]
        constexpr bool has_zoom() const noexcept
        {
            return has_offset(zoom);
        }

        [[nodiscard]]
        static constexpr bool has_offset(pxoff2d offset) noexcept
        {
            return offset.x() || offset.y();
        }
    };

    class mouse_tracker
    {
    public:
        using point2d_type = ui::pointer_event::point2d_type;
        using value_type = point2d_type::value_type;
        using finger_positions_t = static_vector<point2d_type, 2u>;

        [[nodiscard]]
        gesture new_gesture(const ui::mouse_move_event& e) noexcept
        {
            finger_positions_t new_positions{};
            write_positions(new_positions, e, positions_);
            const auto result = make_gesture(positions_, new_positions);
            positions_ = std::move(new_positions);
            return result;
        }

        void finish() noexcept
        {
            positions_.clear();
        }

    private:
        static void write_positions(finger_positions_t& new_positions, const ui::pointer_event& e, const finger_positions_t& order) noexcept
        {
            D_ASSERT(2u == order.capacity());

            switch (e.size())
            {
                case 1u:
                    new_positions.emplace_back(e.pointer(0));
                    break;

                case 2u:
                {
                    auto p0 = e.pointer(0);
                    auto p1 = e.pointer(1);

                    if (order.size())
                    {
                        if (sqr_distance(p1, order[0]) < sqr_distance(p0, order[0]))
                        {
                            std::swap(p0, p1);
                        }
                    }

                    new_positions.emplace_back(p0);
                    new_positions.emplace_back(p1);
                    break;
                }

                default:
                    break;
            };
        }

        [[nodiscard]]
        static constexpr gesture make_gesture(const finger_positions_t& positions0, const finger_positions_t& positions1) noexcept
        {
            constexpr auto move_zoom_gesture = [] (const finger_positions_t& positions0, const finger_positions_t& positions1) noexcept
            {
                constexpr auto center = [] (const finger_positions_t& positions) noexcept
                {
                    D_ASSERT(2u == positions.size());
                    return 0.5 * (positions[0] + positions[1]);
                };

                const auto center0 = center(positions0);
                const auto center1 = center(positions1);
                return center1 - center0;
            };

            constexpr auto zoom_gesture = [] (const finger_positions_t& positions0, const finger_positions_t& positions1) noexcept
            {
                constexpr auto distance = [] (const finger_positions_t& positions) noexcept
                {
                    D_ASSERT(2u == positions.size());
                    const auto d_poistions = narrow2d<point2d<double>>(positions[1]) - positions[0];
                    return vabs(d_poistions);
                };

                const auto distance0 = distance(positions0);
                const auto distance1 = distance(positions1);
                return distance1 - distance0;
            };

            constexpr auto move_gesture = [] (const finger_positions_t& positions0, const finger_positions_t& positions1) noexcept
            {
                D_ASSERT(1u == positions0.size());
                D_ASSERT(1u == positions1.size());
                return as_signed(positions1[0] - positions0[0]);
            };


            D_ASSERT(2u == positions0.capacity());
            D_ASSERT(2u == positions1.capacity());

            if (positions0.size() && positions1.size())
            {
                const auto is_positions0 = (2u == positions0.size());
                const auto is_positions1 = (2u == positions1.size());

                if (is_positions0 && is_positions1)
                {
                    return gesture::instance
                    (
                        move_zoom_gesture(positions0, positions1),
                        zoom_gesture(positions0, positions1)
                    );
                }
                else
                {
                    if (!is_positions0 && !is_positions1)
                    {
                        return gesture::instance(move_gesture(positions0, positions1));
                    }
                }
            }

            return {};
        }

    private:
        finger_positions_t positions_{};
    };

    [[nodiscard]]
    gl::texture2d pix8map_generate(pxsize2d sizes) noexcept
    {
        using pixmap_t = pix8map;

        pixmap_t tex_mem{ sizes };
        if (!tex_mem)
        {
            e_debug("out of memory");
            return {};
        }

        std::default_random_engine content_generator{};

        {
            const auto width = tex_mem.width();
            for (auto line_it : tex_mem)
            {
                for (const auto end = line_it + width; line_it < end; line_it += pixmap_t::alignment)
                {
                    const auto value = content_generator();
                    static_assert(pixmap_t::alignment == sizeof(value));
                    memcpy(line_it, &value, sizeof(value));
                }
            }
        }

        auto texture = gl::create_texture2d(tex_mem);
        if (!texture)
        {
            e_debug("create texture error: {}", glGetError());
            return {};
        }

        return texture;
    }


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
            texture_ = pix8map_generate(egl_.viewport / 4u);
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
                mouse_trace_.finish();
                return std::nullopt;
            }
#endif

            const auto gesture = mouse_trace_.new_gesture(e);

            if (gesture.has_move())
            {
                if (const auto new_area = area_.with_shift(gesture.move, egl_.viewport); new_area != area_)
                {
                    area_ = new_area;
                    need_redraw_ = true;
                }
            }

            if (gesture.has_zoom())
            {
                if (const auto new_area = area_.with_zoom(gesture.zoom); new_area != area_)
                {
                    area_ = new_area;
                    need_redraw_ = true;
                }
            }

            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_up_event&) noexcept
        {
            mouse_trace_.finish();
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
        mouse_tracker mouse_trace_{};
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




