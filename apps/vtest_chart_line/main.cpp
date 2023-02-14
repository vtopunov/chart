#include <numbers>

#include <core/small_vector.h>
#include <core/lerp.h>

#include <px/algorithm.h>

#include <utility/px.h>

#include <widget/run.h>

using widget::window;
using widget::event_result;

namespace
{
    double_t sinc(double_t x) noexcept
    {
        constexpr double_t near_zero_eps{ 0.0004 }; // (eps*120)^(1/4)

        if (abs(x) > near_zero_eps)
        {
            return sin(x) / x;
        }
        else
        {
            return 1 - x * x / 6;
        }
    }

    using f64point2d = point2d<double_t>;

    bool sinc_vector_initialize(small_vector<f64point2d>& v) noexcept
    {
        constexpr auto size = 800_uz;

        if (!v.try_reserve(size))
        {
            e_debug("sinc_vector_initialize: out of memory");
            return false;
        }

        constexpr auto abscissa_max = 8 * std::numbers::pi_v<double_t>;
        constexpr num_range abscissa_range{ -abscissa_max, abscissa_max };
        constexpr num_range index_range{ 0_uz, size - 1_uz };
        constexpr auto abscissa = lerp(index_range, abscissa_range);

        for (size_t i = index_range._0; i <= index_range._1; ++i)
        {
            const auto x = abscissa(i);
            v.emplace_back(x, sinc(x));
        }

        return true;
    };

    using f64range = num_range<double_t>;

    void expand(f64range& range, double_t value) noexcept
    {
        if (std::isfinite(value))
        {
            min_eq(range._0, value);
            max_eq(range._1, value);
        }
    }

    bool range_is_valid(const f64range& range) noexcept
    {
        return std::isfinite(range._0)
            && std::isfinite(range._1)
            && range._1 > range._0
            && std::isnormal(range.length());
    }

    using f64point2drange = point2d<f64range>;

    template<size_t axis>
    void correct(f64point2drange& ranges) noexcept
    {
        constexpr f64range default_range{ 0.0, 1.0 };

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

    f64point2drange calculate_values_range(span<const f64point2d> line) noexcept
    {
        constexpr num_range range0
        {
            numeric_max_v<double_t>,
            numeric_min_v<double_t>
        };

        constexpr auto point_range0 = fill_to<point2d>(range0);

        f64point2drange values_range{ point_range0 };

        for (const auto& pt : line)
        {
            expand(values_range.ref_x(), pt.x());
            expand(values_range.ref_y(), pt.y());
        }

        correct<0>(values_range);
        correct<1>(values_range);

        return values_range;
    }

    constexpr f64point2drange calculate_pix_range(pxsize2d sizes) noexcept
    {
        return
        {
            f64range{ 0.0, sizes.width() - 1.0 },
            f64range{ 0.0, sizes.height() - 1.0 },
        };
    }

    struct coordinate_transformation : vec2<polynomial2<double_t>>
    {
        template<class Pt>
        constexpr f64point2d operator () (const Pt& pt) const noexcept
        {
            return { _0(pt._0), _1(pt._1) };
        }
    };

    constexpr coordinate_transformation calculate_coordinate_transformation
    (
        const f64point2drange& from,
        const f64point2drange& to
    ) noexcept
    {
        return
        {
            lerp(from.cref_x(), to.cref_x()),
            lerp(inverse(from.cref_y()), to.cref_y())
        };
    }


    template<class Transform>
    constexpr void draw_polyline
    (
        const pix8span image,
        span<const f64point2d> values,
        Transform value2px
    ) noexcept
    {
        if (D_LIKELY(values.size())) D_ATTRIB_LIKELY
        {
            auto p0 = value2px(values[0]);

            for (const auto& p : values.subspan(1u))
            {
                const auto p1 = value2px(p);
                px::draw_antialiasing_line(image, p0.x(), p0.y(), p1.x(), p1.y());
                p0 = p1;
            }
        }
    }

    constexpr void draw_chart_polyline
    (
        const pix8span image,
        span<const f64point2d> values,
        const f64point2drange& values_range
    ) noexcept
    {
        draw_polyline(image, values, calculate_coordinate_transformation
        (
            values_range,
            calculate_pix_range(image.sizes())
        ));
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
            static constexpr auto line_color = gl::colors::blue_f;
            static constexpr pxsize2d max_sizes{ fill_vec2(numeric_max_v<pxside_t>) };

            class values_container : public small_vector<f64point2d>
            {
                static constexpr auto nan_value = numeric_nan_v<double_t>;
                static constexpr auto nan_values_range = fill_to<point2d>(fill_to<num_range>(nan_value));

            public:
                f64point2drange values_range() const noexcept
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
                mutable f64point2drange values_range_cache_{ nan_values_range };
            };

            pxrectangle geometry{ .position{}, .sizes{ max_sizes } };
            values_container values{};
            gl::texture2d texture_cache{};

            bool operator () (window_configuration& cfg) noexcept
            {
                texture_cache = gl::create_texture2d();
                if (!texture_cache)
                {
                    e_debug("chart_line: create texture error: {}", glGetError());
                    return false;
                }

                cfg.build()
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
                if (const auto view_sizes = clamp_sizes(geometry, w.user_sizes()); view_sizes != sizes(texture_cache))
                {
                    const auto image = px::zeros_pix8space(w.temp_buffer_view(), view_sizes);
                    draw_chart_polyline(image, values, values.values_range());
                    texture_cache = gl::write(std::move(texture_cache), image);
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


        chart_line line;

        bool operator () (window_configuration&) noexcept
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
    return widget::run<chart_widget>(nullptr);
}




