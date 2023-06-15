#include <numbers>

#include <core/small_vector.h>
#include <core/lerp.h>
#include <core/round.h>

#include <utility/px.h>

#include <widget/run.h>

#include <debug/debug.h>

#include <px/algorithm.h>


using namespace std::string_view_literals;

using widget::window;
using widget::event_result;

using px::real_t;
using px::point2d_real;

namespace
{
    real_t sinc(real_t x) noexcept
    {
        constexpr real_t near_zero_eps{ 0.0004 }; // (eps*120)^(1/4)

        if (abs(x) > near_zero_eps)
        {
            return sin(x) / x;
        }
        else
        {
            return 1 - x * x / 6;
        }
    }

    bool sinc_vector_initialize(small_vector<point2d_real>& v) noexcept
    {
        constexpr auto size = 800_uz;

        if (!v.try_reserve(size))
        {
            e_debug("sinc_vector_initialize: out of memory");
            return false;
        }

        constexpr auto abscissa_max = 8 * std::numbers::pi_v<real_t>;
        constexpr num_range abscissa_range{ -abscissa_max, abscissa_max };
        constexpr num_range index_range{ 0_uz, size - 1_uz };
        constexpr auto abscissa = lerp(index_range, abscissa_range);

        for (size_t i = index_range._0; i <= index_range._1; ++i)
        {
            const auto x = abscissa(i);
            v.emplace_back(x, sin(x));
        }

        return true;
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
    void correct(point2drange_real& ranges) noexcept
    {
        constexpr range_real default_range{ 0.0, 1.0 };

        auto& range = get<axis>(ranges);
        if (!range_is_valid(range))
        {
            range = default_range;

            constexpr vec2 axis_letters{ 'X', 'Y' };
            w_debug
            (
                "invalid {} axis range: [{}, {}]",
                get<axis>(axis_letters),
                range._0,
                range._1
            );
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

        correct<0>(values_range);
        correct<1>(values_range);

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
        template<class Pt>
        constexpr point2d_real operator () (const Pt& pt) const noexcept
        {
            return { _0(pt._0), _1(pt._1) };
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
        if (D_LIKELY(values.size())) D_ATTRIB_LIKELY
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
    constexpr pxsize2d clamp_sizes(const rectangle<Pos, Sz>& r, pxsize2d max_sizes) noexcept
    {
        using overpxoff_t = int64_t;
        static_assert(sizeof(overpxoff_t) > sizeof(Pos));
        static_assert(sizeof(overpxoff_t) > sizeof(Sz));
        static_assert(sizeof(overpxoff_t) > sizeof(pxside_t));

        constexpr auto clamp_len = [] (overpxoff_t position, overpxoff_t len, overpxoff_t maxlen) noexcept
        {
            return narrow_cast<pxside_t>(std::min(position + len, maxlen) - position);
        };

        return
        {
            clamp_len(r.x(), r.width(), max_sizes.width()),
            clamp_len(r.y(), r.height(), max_sizes.height())
        };
    }

    struct chart_widget
    {
        struct chart_line
        {
            static constexpr auto background_color = gl::colors::white_f;
            static constexpr auto line_color = gl::colors::red_f;
            static constexpr pxsize2d max_sizes{ fill_vec2(numeric_max_v<pxside_t>) };

            class values_container : public small_vector<point2d_real>
            {
                static constexpr auto nan_value = numeric_nan_v<real_t>;
                static constexpr auto nan_values_range = fill_to<point2d>(fill_to<num_range>(nan_value));

            public:
                point2drange_real values_range() const noexcept
                {
                    if (need_to_update_cache())
                    {
                        values_range_cache_ = calculate_values_range(*this);
                    }

                    return values_range_cache_;
                }

            private:
                bool need_to_update_cache() const noexcept
                {
                    return std::isnan(values_range_cache_._0._0);
                }

            private:
                mutable point2drange_real values_range_cache_{ nan_values_range };
            };

            pxrectangle geometry{ .position{}, .sizes{ max_sizes } };
            values_container values{};
            gl::texture2d texture_cache{};

            bool operator () (widget_initializer& ini) noexcept
            {
                texture_cache = gl::create_texture2d();
                if (!texture_cache)
                {
                    e_debug("chart_line: create texture error: {}", glGetError());
                    return false;
                }

                ini.cfg()
                    .gray_texture_mix_color_shdr()
                    .colored_rectangle_shdr()
                    .pix8_temp_buffer();

                return true;
            }

            constexpr event_result operator () (const ui::size_event&) noexcept
            {
                return event_result::redraw;
            }

            void draw(const window& w) noexcept
            {
                if (const auto chart_sizes = clamp_sizes(geometry, w.user_sizes()); chart_sizes != sizes(texture_cache))
                {
                    const auto chart_image = px::zeros_pix8space(w.temp_buffer_view(), chart_sizes);
                    draw_polyline(chart_image, values, calculate_coordinate_transformation
                    (
                        values.values_range(),
                        calculate_pix_range(chart_sizes)
                    ));
                    texture_cache = gl::write(std::move(texture_cache), chart_image);
                }

                const pxrectangle view_geometry
                {
                    .position{ geometry.position },
                    .sizes{ sizes(texture_cache) }
                };

                w.shaders.colored_rectangle.draw(view_geometry, background_color);
                w.shaders.gray_texture_mix_color.draw(view_geometry.position, texture_cache, line_color);
            }
        };

        chart_line line{};

        bool operator () (const widget_initializer&) noexcept
        {
            if (!sinc_vector_initialize(line.values))
            {
                e_debug("initialize chart error");
                return false;
            }

            return true;
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(line);
        }
    };
}


int main() noexcept
{
    auto window = widget::window_builder{}
        .sizes(1001_px, 157_px)
        .command_show(ui::show_command::normal)
        .build();

    return widget::run<chart_widget>(window);
}