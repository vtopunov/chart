#include <random>
#include <variant>

#include <core/round.h>
#include <core/static_vector.h>

#include <debug/debug.h>

#include <utility/shaders_library.h>
#include <egl_ui/egl_ui_owner.h>


namespace
{
    template<class L, class R>
    [[nodiscard]] constexpr decltype(auto) sqr_distance(const point2d<L>& p0, const point2d<R>& p1) noexcept
    {
        const auto dpt = as_signed(p0 - p1);
        return to_unsigned_or(dpt.x() * dpt.x()) + to_unsigned_or(dpt.y() * dpt.y());
    };

    using zoom_value_t = uint64_t;
    static_assert(sizeof(zoom_value_t) > sizeof(pxside_t));

    [[nodiscard]] constexpr zoom_value_t zoom_value_for_full_srceen(pxsize2d viewport) noexcept
    {
        return static_cast<zoom_value_t>(viewport.width()) * viewport.height();
    }

    struct figure_zoom
    {
        using value_type = zoom_value_t;

        value_type value{ 0u };

        [[nodiscard]]
        constexpr figure_zoom with_clamp(pxsize2d viewport) const noexcept
        {
            constexpr auto max_zoom = [] (pxsize2d viewport) noexcept -> value_type
            {
                constexpr auto px_digits = numeric_digits_v<pxoff_t>;
                constexpr auto gl_max_mantissa = numeric_max_v<pxoff_t> >> (px_digits - std::min(numeric_digits_v<GLfloat>, px_digits));
                const auto [min_dim, max_dim] = std::minmax(viewport.width(), viewport.height());
                return std::min(min_dim, gl_max_mantissa / max_dim) * zoom_value_for_full_srceen(viewport);
            };

            constexpr auto min_zoom = [] (pxsize2d viewport) noexcept -> value_type
            {
                return std::max(viewport.width(), viewport.height());
            };

            const auto new_value = std::clamp(value, min_zoom(viewport), max_zoom(viewport));
            return { .value{ new_value } };
        }

        [[nodiscard]]
        figure_zoom with_increase(double rot) const noexcept
        {
            constexpr double mul{ 0.05 };
            const auto dvalue = static_cast<value_type>(value * pow(mul, abs(rot)));
            const auto new_value = signbit(rot) ? value - dvalue : value + dvalue;
            return { .value{ new_value } };
        }

        [[nodiscard]]
        figure_zoom with_multiplier(double mul) const noexcept
        {
            D_ASSERT(!signbit(mul));
            const auto new_value = static_cast<value_type>(value * abs(mul));
            return { .value{ new_value } };
        }

        [[nodiscard]]
        constexpr pxsize2d operator () (pxsize2d viewport) const noexcept
        {
            return narrow2d_cast<pxsize2d>(value / viewport.height(), value / viewport.width());
        }

        [[nodiscard]]
        constexpr bool operator == (const figure_zoom&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const figure_zoom&) const noexcept = default;
    };

    [[nodiscard]]
    constexpr figure_zoom full_screen_zoom(pxsize2d viewport) noexcept
    {
        return { .value{ zoom_value_for_full_srceen(viewport) } };
    }

    struct figure_center_position
    {
        pxoff2d position{};

        [[nodiscard]]
        static constexpr figure_center_position instance(pxsize2d viewport) noexcept
        {
            return { .position{ narrow2d_cast<pxoff2d>(viewport / 2_px) } };
        }

        [[nodiscard]]
        constexpr figure_center_position with_clamp(pxsize2d fig_sizes, pxsize2d viewport) const noexcept
        {
            const auto min_position = -narrow2d_cast<pxoff2d>((fig_sizes + fill_vec2(1_px)) / 2_px);
            const auto max_position = narrow2d_cast<pxoff2d>((fig_sizes + 2_px * viewport) / 2_px);

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
            return (2 * position - narrow2d_cast<pxoff2d>(fig_sizes)) / 2;
        }

        [[nodiscard]]
        constexpr bool operator == (const figure_center_position&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const figure_center_position&) const noexcept = default;
    };


    struct figure_area
    {
        figure_center_position position{};
        figure_zoom zoom{};

        [[nodiscard]]
        constexpr rectangle<px::pxoff_t> geometry(pxsize2d viewport) const noexcept
        {
            const auto fig_sizes = zoom(viewport);
            return
            {
                .position{ position.left_top(fig_sizes) },
                .sizes{ fig_sizes }
            };
        }

        [[nodiscard]]
        constexpr figure_area with_shift(pxoff2d shift, pxsize2d viewport) const noexcept
        {
            return
            {
                .position{ position.with_shift(shift).with_clamp(zoom(viewport), viewport) },
                .zoom{ zoom }
            };
        }

        [[nodiscard]]
        figure_area with_zoom(figure_zoom new_zoom, pxsize2d viewport) const noexcept
        {
            return
            {
                .position{ position.with_clamp(new_zoom(viewport), viewport) },
                .zoom{ new_zoom }
            };
        }

        [[nodiscard]]
        figure_area with_zoom_increase(double rot, pxsize2d viewport) const noexcept
        {
            const auto new_zoom = zoom.with_increase(rot).with_clamp(viewport);
            return with_zoom(new_zoom, viewport);
        }

        [[nodiscard]]
        figure_area with_zoom_multiplier(double mul, pxsize2d viewport) const noexcept
        {
            const auto new_zoom = zoom.with_multiplier(mul).with_clamp(viewport);
            return with_zoom(new_zoom, viewport);
        }

        [[nodiscard]]
        constexpr bool operator == (const figure_area&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const figure_area&) const noexcept = default;
    };


    struct gesture
    {
        pxoff2d move{ 0, 0 };
        double zoom{ numeric_nan_v<double> };

        template<class T> [[nodiscard]]
        gesture with_move(const point2d<T>& p) const noexcept
        {
            return
            {
                .move
                {
                    trunc_cast<pxoff_t>(p.x()),
                    trunc_cast<pxoff_t>(p.y())
                },
                .zoom{ zoom }
            };
        }

        [[nodiscard]]
        constexpr bool has_move() const noexcept
        {
            return move.x() || move.y();
        }

        [[nodiscard]]
        bool has_zoom() const noexcept
        {
            return std::isnormal(zoom);
        }
    };

    class mouse_tracker
    {
    public:
        using point2d_type = ui::pointer_event::point2d_type;
        using value_type = point2d_type::value_type;
        using trace_slice_t = static_vector<point2d_type, 2u>;

        [[nodiscard]]
        gesture new_gesture(const ui::mouse_move_event& e) noexcept
        {
            trace_slice_t new_trace{};
            write_trace_slice(new_trace, e, trace_);
            const auto result = make_gesture(trace_, new_trace);
            trace_ = std::move(new_trace);
            return result;
        }

        void finish() noexcept
        {
            trace_.clear();
        }

    private:
        [[nodiscard]]
        static constexpr point2d<double> center(const trace_slice_t& trace) noexcept
        {
            D_ASSERT(2u == trace.size());
            return 0.5 * (trace[0] + trace[1]);
        }

        [[nodiscard]]
        static double diagonal_length(const trace_slice_t& trace) noexcept
        {
            D_ASSERT(2u == trace.size());
            return sqrt(sqr_distance(trace[0], trace[1]));
        }

        static void write_trace_slice(trace_slice_t& new_trace, const ui::pointer_event& e, const trace_slice_t& order) noexcept
        {
            D_ASSERT(2u == order.capacity());

            switch (e.size())
            {
                case 1u:
                    new_trace.emplace_back(e.pointer(0));
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

                    new_trace.emplace_back(p0);
                    new_trace.emplace_back(p1);
                    break;
                }

                default:
                    break;
            };
        }

        [[nodiscard]]
        static constexpr gesture make_gesture(const trace_slice_t& trace0, const trace_slice_t& trace1) noexcept
        {
            D_ASSERT(2u == trace0.capacity());;
            D_ASSERT(2u == trace1.capacity());;

            gesture result{};

            if (trace0.size() && trace1.size())
            {
                const auto is_trace01 = (2u == trace0.size());
                const auto is_trace11 = (2u == trace1.size());

                if (is_trace01 && is_trace11)
                {
                    {
                        const auto c0 = center(trace0);
                        const auto c1 = center(trace1);
                        result = result.with_move(c1 - c0);
                    }

                    {
                        constexpr auto min_diagonal_length = 0.71;
                        const auto len0 = diagonal_length(trace0);
                        if (len0 > min_diagonal_length)
                        {
                            const auto len1 = diagonal_length(trace1);
                            if (len1 > min_diagonal_length)
                            {
                                result.zoom = len1 / len0;
                            }
                        }

                    }
                }
                else
                {
                    if (!is_trace01 && !is_trace11)
                    {
                        result = result.with_move(as_signed(trace1[0] - trace0[0]));
                    }
                }
            }

            return result;
        }

    private:
        trace_slice_t trace_{};
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
            if (const auto new_area = area_.with_zoom_increase(e.rot(), egl_.viewport); new_area != area_)
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
                if (const auto new_area = area_.with_zoom_multiplier(gesture.zoom, egl_.viewport); new_area != area_)
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

            shaders_.draw(area_.geometry(egl_.viewport));
        }

        [[nodiscard]]
        static figure_area default_area(pxsize2d viewport) noexcept
        {
            return
            {
                .position{ figure_center_position::instance(viewport) },
                .zoom{ full_screen_zoom(viewport).with_multiplier(0.5) }
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




