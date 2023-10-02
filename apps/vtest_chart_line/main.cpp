#include <cmath>

#include <core/small_vector.h>
#include <core/lerp.h>
#include <core/round.h>

#include <px/algorithm.h>

#include <utility/px.h>

#include <widget/run.h>
#include <widget/button.h>
#include <widget/gesture_cache.h>

using namespace std::string_view_literals;

using widget::event_result;

using px::real_t;
using px::point2d_real;


namespace
{
    using stretchable_pxrectangle = ::rectangle<pxside_t, pxoff_t>;

    constexpr auto n_points = 400_uz;

    void sin_vector_initialize(small_vector<point2d_real>& v) noexcept
    {
        constexpr auto pi = 3.141592653589793238462643383279502884L;
        constexpr auto abscissa_max = static_cast<real_t>(12.0 * pi);
        constexpr num_range abscissa_range{ -abscissa_max, abscissa_max };
        constexpr num_range index_range{ 0_uz, n_points - 1_uz };
        constexpr auto abscissa = lerp(index_range, abscissa_range);

        D_ASSERT(0u == v.size());
        D_ASSERT(n_points <= v.capacity());
        for (size_t i = index_range._0; i <= index_range._1; ++i)
        {
            const auto x = abscissa(i);
            v.emplace_back(x, sin(x));
        }
    };

    using range_real = num_range<real_t>;

    void expand(range_real& range, real_t value) noexcept
    {
        if (std::isfinite(value))
        {
            min_eq(range._0, value);
            max_eq(range._1, value);
        }
    }

    bool range_is_valid(const range_real& range) noexcept
    {
        return std::isfinite(range._0)
            && std::isfinite(range._1)
            && range._1 > range._0
            && std::isnormal(range.length());
    }

    using point2drange_real = point2d<range_real>;

    template<size_t axis>
    void correct(point2drange_real& ranges, bool w_output) noexcept
    {
        constexpr range_real default_range{ 0.0, 1.0 };

        auto& range = get<axis>(ranges);
        if (!range_is_valid(range))
        {
            if (w_output)
            {
                constexpr vec2 axis_letters{ 'X', 'Y' };
                w_debug
                (
                    "invalid {} axis range: [{}, {}]",
                    get<axis>(axis_letters),
                    range._0,
                    range._1
                );
            }

            range = default_range;
        }
    }

    using const_span_point2d_real = span<const point2d_real>;

    point2drange_real calculate_values_range(const_span_point2d_real line) noexcept
    {
        constexpr num_range range0
        {
            numeric_max_v<real_t>,
            numeric_min_v<real_t>
        };

        constexpr auto point_range0 = fill_to<point2d>(range0);

        point2drange_real values_range{ point_range0 };

        for (const auto& pt : line)
        {
            expand(values_range.ref_x(), pt.x());
            expand(values_range.ref_y(), pt.y());
        }

        {
            const auto w_output = !!line.size();
            correct<0>(values_range, w_output);
            correct<1>(values_range, w_output);
        }

        return values_range;
    }

    constexpr point2drange_real calculate_pix_range(pxsize2d sizes) noexcept
    {
        return
        {
            range_real{ 0.0, sizes.width() - 1.0 },
            range_real{ 0.0, sizes.height() - 1.0 },
        };
    }

    struct coordinate_transformation : vec2<polynomial2<real_t>>
    {
        template<class T>
        constexpr point2d_real operator () (const vec2<T>& v) const noexcept
        {
            return { _0(v._0), _1(v._1) };
        }
    };

    constexpr coordinate_transformation calculate_coordinate_transformation
    (
        const point2drange_real& from,
        const point2drange_real& to
    ) noexcept
    {
        return
        {
            lerp(from.cref_x(), to.cref_x()),
            lerp(inverse(from.cref_y()), to.cref_y())
        };
    }

    constexpr void draw_polyline
    (
        const pix8span image,
        const const_span_point2d_real values,
        const coordinate_transformation value2px
    ) noexcept
    {
        if (values.size()) [[likely]]
        {
            auto cached_result = px::invalid_antialiasing_line_result_v;
            auto p0 = value2px(values.front());
            for (const auto& p : values.subspan(1u))
            {
                const auto p1 = value2px(p);
                const auto result = px::draw_antialiasing_line(image, p0, p1, cached_result);
                cached_result = result;
                p0 = p1;
            }
        }
    }

    template<class Pos, class Sz>
    constexpr pxsize2d clamp_sizes(const rectangle<Pos, Sz>& r, pxsize2d window_sizes) noexcept
    {
        using overpxoff_t = int64_t;
        static_assert(std::is_signed_v<Sz>);
        static_assert(sizeof(overpxoff_t) > sizeof(Pos));
        static_assert(sizeof(overpxoff_t) > sizeof(Sz));
        static_assert(sizeof(overpxoff_t) > sizeof(pxside_t));

        constexpr auto clamp_len = [] (overpxoff_t position, overpxoff_t fixlen, overpxoff_t len) noexcept
        {
            len -= position;
            if (len < 0LL) [[unlikely]]
                return 0_px;

                if (fixlen <= 0LL)
                {
                    len += fixlen;

                    if (len < 0LL) [[unlikely]]
                        return 0_px;
                }
                else
                {
                    if (fixlen < len)
                    {
                        len = fixlen;
                    }
                }

                return narrow<pxside_t>(len);
        };

        return
        {
            clamp_len(r.x(), r.width(), window_sizes.width()),
            clamp_len(r.y(), r.height(), window_sizes.height())
        };
    }

    struct chart_widget
    {
        struct chart_line
        {
            using container_of_points = small_vector<point2d_real>;

            static constexpr auto background_color = gl::colors::white_f;
            static constexpr auto line_color = gl::colors::red_f;

            class range_cache
            {
                static constexpr num_range invalid_range{ numeric_max_v<real_t>, numeric_lowest_v<real_t> };
                static constexpr auto invalid_cache = fill_to<point2d>(invalid_range);

            public:
                point2drange_real update_and_get(const_span_point2d_real line) noexcept
                {
                    if (need_to_update())
                    {
                        cache_ = calculate_values_range(line);
                    }

                    return cache_;
                }

                constexpr void clear() noexcept
                {
                    cache_ = invalid_cache;
                }

                constexpr bool need_to_update() const noexcept
                {
                    return invalid_range._0 == cache_._0._0;
                }

            private:
                point2drange_real cache_{ invalid_cache };
            };

            stretchable_pxrectangle geometry{};
            container_of_points points{};
            gl::texture2d texture_cache{};
            range_cache values_range_cache{};

            bool operator () (const widget::window&) noexcept
            {
                texture_cache = gl::create_texture2d();
                if (!texture_cache)
                {
                    e_debug("chart_line: create texture error: {}", glGetError());
                    return false;
                }

                return true;
            }

#ifdef D_OS_WINDOWS
            event_result operator () (const ui::size_event&) noexcept
            {
                //debug("size event: {}x{}", e.width(), e.height());
                return event_result::redraw;
            }
#endif

            void clear_texture_cache() noexcept
            {
                texture_cache = gl::sizes(std::move(texture_cache), 0_px, 0_px);
            }

            void clear_cache() noexcept
            {
                values_range_cache.clear();
                clear_texture_cache();
            }

            using mouse_move_event_type = widget::mouse_move_event<
                widget::gesture_cache,
                widget::content_size2d
            >;

            using redraw_event_type = widget::redraw_event<
                widget::shader::gray_texture_mix_color,
                widget::shader::colored_rectangle,
                buffer_view,
                widget::content_size2d
            >;

            void operator () (mouse_move_event_type e) noexcept
            {
                const auto sizes = e.get<widget::content_size2d>();
                debug("mouse_move: ({}, {}), {}x{}", e.x(), e.y(), sizes.width(), sizes.height());

                if (const auto& gesture_cache = e.get<widget::gesture_cache>())
                {
                    if (const auto move = gesture_cache.move())
                    {
                        debug("move: ({}, {})", move.x(), move.y());
                    }

                    if (const auto zoom = gesture_cache.zoom())
                    {
                        debug("zoom: ({}, {})", zoom.x(), zoom.y());
                    }
                }
            }

            void operator () (redraw_event_type e) noexcept
            {
                if (const auto chart_sizes = clamp_sizes(geometry, e.get<widget::content_size2d>()); chart_sizes != sizes(texture_cache))
                {
                    //debug("redraw: {}x{}", chart_sizes.width(), chart_sizes.height());

                    const auto chart_image = px::zeros_pix8space(e.get<buffer_view>(), chart_sizes);
                    draw_polyline(chart_image, points, calculate_coordinate_transformation
                    (
                        values_range_cache.update_and_get(points),
                        calculate_pix_range(chart_sizes)
                    ));
                    texture_cache = gl::write(std::move(texture_cache), chart_image);
                }

                const pxrectangle view_geometry
                {
                    .position{ geometry.position },
                    .sizes{ sizes(texture_cache) }
                };

                e.get<widget::shader::colored_rectangle>().draw(view_geometry, background_color);
                e.get<widget::shader::gray_texture_mix_color>().draw(view_geometry.position, texture_cache, line_color);
            }

            template<class Fn>
            decltype(auto) apply(Fn fn) noexcept
            {
                return fn
                (
                    widget::ex_context_v<redraw_event_type>,
                    widget::ex_context_v<widget::gesture_cache>
                );
            }
        };

        static constexpr pxpoint2d x00{ 20_px, D_CONDITIONAL_OS_ANDROID(60_px, 20_px) };
        static constexpr pxsize2d button_sizes{ 120_px, D_CONDITIONAL_OS_ANDROID(60_px, 40_px) };

        template<size_t n>
        static constexpr point2d button_position_v
            = x00 + point2d{ narrow<pxside_t>(n * (5_px + button_sizes.width())), 0_px };

        widget::button b_plot
        {
            .geometry
            {
                .position{ button_position_v<0u> },
                .sizes{ button_sizes }
            },
            .text{ u8"Построить" }
        };

        widget::button b_clear
        {
            .geometry
            {
                .position{ button_position_v<1u> },
                .sizes{ button_sizes }
            },
            .text{ u8"Очистить" }
        };

        widget::button b_exit
        {
            .geometry
            {
                .position{ button_position_v<2u> },
                .sizes{ button_sizes }
            },
            .text{ u8"Выход" }
        };

        chart_line line
        {
            .geometry
            {
                .position{ x00 + point2d{ 0_px, button_sizes.height() + 15_px } },
                .sizes{-20_pxz, -20_pxz}
            }
        };

        chart_line::container_of_points points{};

        bool operator () (os::const_module_handle_t app) noexcept
        {
            if (!points.try_reserve(n_points)) [[unlikely]]
            {
                e_debug("chart line values: out of memory");
                return false;
            }

            sin_vector_initialize(points);

            b_plot.clicked = [this] () noexcept
            {
                if (!line.points.size())
                {
                    line.points = std::move(points);
                }

                line.clear_cache();
            };

            b_clear.clicked = [this] () noexcept
            {
                if (line.points.size())
                {
                    points = std::move(line.points);
                    line.clear_cache();
                }
            };

            b_exit.clicked = [app] () noexcept
            {
                ui::quit(app);
            };

            return true;
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(b_plot, b_clear, b_exit, line);
        }
    };
}


int app_main(os::module_handle_t app) noexcept
{
    return widget::run<chart_widget>(app);
}