#pragma once

#include <px/pixspan.h>


namespace px
{
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_use_bitwise_and_to_check_enum_flags);
    D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized);
    D_WARNING_DISABLE_MSVC(W_converting_from_floating_point_to_unsigned_integral);
    D_WARNING_DISABLE_MSVC(W_do_not_use_pointer_arithmetic);
    D_WARNING_DISABLE_MSVC(W_use_not_null);

    namespace private_detail_algorithm
    {
        namespace private_detail_constants
        {
            constexpr real_t _0_0{ 0.0 };
            constexpr real_t _0_5{ 0.5 };
            constexpr real_t _256_0{ 256.0 };
        }

        using namespace private_detail_constants;


        namespace private_detail_antialiasing_line
        {
            using namespace private_detail_constants;

            constexpr auto eps = _0_5 / _256_0;
            constexpr auto _256_plus = _256_0 + eps;
            constexpr auto _256_0_0_bound = -255.0 - eps;

            template<class T>
            [[nodiscard]] constexpr T sign_if_not(bool cond, T value) noexcept
            {
                return (cond) ? value : -value;
            };

            template<class T>
            [[nodiscard]] constexpr T sign_if_not_likely(bool cond, T value) noexcept
            {
                if (cond) [[likely]]
                {
                    return value;
                }
                else
                {
                    return -value;
                }
            };

            [[nodiscard]] constexpr ptrdiff_t z_round_unsafe(real_t value) noexcept
            {
                return static_cast<ptrdiff_t>(value + _0_5);
            };

            [[nodiscard]] constexpr ptrdiff_t z_round
            (
                const double_t value,
                const ptrdiff_t bound0,
                const ptrdiff_t bound1
            ) noexcept
            {
                {
                    constexpr auto near_zero_neg = -1.5 + eps;
                    static_assert(0_z == z_round_unsafe(near_zero_neg));
                    static_assert(-1_z == z_round_unsafe(-1_z + near_zero_neg));

                    const auto bound_d = bound0 + near_zero_neg;
                    if (value < bound_d) [[unlikely]]
                    {
                        return bound0;
                    }
                }

                {
                    constexpr auto near_zero_p = -_0_5 - eps;
                    static_assert(0_z == z_round_unsafe(near_zero_p));
                    static_assert(0_z == z_round_unsafe(1_z + near_zero_p));

                    const auto bound_d = bound1 + near_zero_p;
                    if (bound_d < value) [[unlikely]]
                    {
                        return bound1;
                    }
                }

                return z_round_unsafe(value);
            };

            [[nodiscard]] constexpr vec2<ptrdiff_t> round_range(bool direction_is_inc, npx_t size, real_t v0, real_t v1) noexcept
            {
                const auto bound0 = 0_z - !direction_is_inc;
                const auto bound1 = bound0 + narrow<ptrdiff_t>(size);

                return
                {
                    z_round(v0, bound0, bound1),
                    z_round(v1, bound0, bound1)
                };
            };

            struct position_shade
            {
                using index_shade_t = uint64_t;

                index_shade_t index_shade;

                [[nodiscard]] constexpr size_t index() const noexcept
                {
                    constexpr auto shade_bits = 8u * sizeof(luminance_t);
                    return static_cast<size_t>(index_shade >> shade_bits);
                }

                [[nodiscard]] constexpr luminance_t shade() const noexcept
                {
                    constexpr auto shade_mask = numeric_max_v<luminance_t>;
                    return static_cast<luminance_t>(index_shade & shade_mask);
                }

                [[nodiscard]] static constexpr position_shade instance_from_real(real_t code) noexcept
                {
                    return { static_cast<index_shade_t>(code) };
                }
            };

            template<class T>
            [[nodiscard]] constexpr T inv(T value) noexcept
            {
                return static_cast<T>(~value);
            }

            using antialiasing_line_result_types_underlying_t = uint8_t;

            enum class antialiasing_line_result_types : antialiasing_line_result_types_underlying_t
            {
                along_x,
                along_y,
                invalid = numeric_max_v<antialiasing_line_result_types_underlying_t>
            };

            struct antialiasing_line_result
            {
                using types = antialiasing_line_result_types;

                luminance_t* p;
                luminance_t shade;
                types type;

                constexpr void join_along_y(ptrdiff_t line_size_z, luminance_t* new_p, luminance_t new_shade) const noexcept
                {
                    if (types::along_x == type)
                    {
                        const auto dp = new_p - p;

                        if (dp == (2_z * line_size_z + 1_z))
                        {
                            const auto inv_shade = inv(shade);
                            const auto mean_shade = narrow<luminance_t>((inv_shade + new_shade) / 2u);
                            new_p[-line_size_z] = mean_shade;
                            return;
                        }

                        if (dp == (1_z - line_size_z))
                        {
                            const auto mean_shade = narrow<luminance_t>((shade + new_shade) / 2u);
                            p[1_z] = mean_shade;
                            return;
                        }
                    }
                }

                constexpr void join_along_x(ptrdiff_t line_size_z, luminance_t* new_p, luminance_t new_shade) const noexcept
                {
                    if (types::along_y == type)
                    {
                        const auto dp = new_p - p;

                        if (dp == (line_size_z + 2_z))
                        {
                            const auto inv_shade = inv(shade);
                            const auto mean_shade = narrow<luminance_t>((inv_shade + new_shade) / 2u);
                            new_p[-1_z] = mean_shade;
                            return;
                        }

                        if (dp == (2_z - 2_z * line_size_z))
                        {
                            const auto inv_shade = inv(shade);
                            const auto inv_new_shade = inv(new_shade);
                            const auto mean_shade = narrow<luminance_t>((inv_shade + inv_new_shade) / 2u);
                            new_p[line_size_z - 1_z] = mean_shade;
                            return;
                        }
                    }
                }
            };

            constexpr antialiasing_line_result invalid_antialiasing_line_result
            {
                .p{ nullptr },
                .shade{ 0u },
                .type{ antialiasing_line_result_types::invalid }
            };
        }
    }

    using private_detail_algorithm::private_detail_antialiasing_line::antialiasing_line_result;
    using private_detail_algorithm::private_detail_antialiasing_line::invalid_antialiasing_line_result;

    constexpr antialiasing_line_result draw_antialiasing_line
    (
        const lumpixspan image,
        const real_point2d p0,
        const real_point2d p1,
        antialiasing_line_result joiner
    ) noexcept
    {
        using namespace private_detail_algorithm::private_detail_antialiasing_line;

        constexpr auto invalid_result = invalid_antialiasing_line_result;

        const auto line_size_z = narrow<ptrdiff_t>(image.line_size());

        const auto [dx, dy] = p1 - p0;

        const auto direction_is_inc_x = _0_0 < dx;
        const auto direction_is_inc_y = _0_0 < dy;

        const auto along_y = sign_if_not_likely(direction_is_inc_x, dx) < sign_if_not(direction_is_inc_y, dy);

        if (along_y)
        {
            if (const auto [y0_z, y1_z] = round_range(direction_is_inc_y, image.height(), p0.y(), p1.y()); y0_z != y1_z) [[likely]]
            {
                const auto gradient_x = dx / dy;

                auto _256_xf = _256_0 * (p0.x() + gradient_x * (y0_z - p0.y()));
                const auto _256_dxf = sign_if_not(direction_is_inc_y, _256_0) * gradient_x;
                const auto _256_xf_aa_bound = _256_0 * image.width() - _256_plus;

                auto p_y = image.data() + y0_z * line_size_z;
                const auto end_p_y = image.data() + y1_z * line_size_z;
                const auto d_p_y = sign_if_not(direction_is_inc_y, line_size_z);

                if (_0_0 <= _256_xf && _256_xf < _256_xf_aa_bound) [[likely]]
                {
                    const auto index_shade = position_shade::instance_from_real(_256_xf);
                    const auto p_x = p_y + index_shade.index();
                    const auto shade = index_shade.shade();

                    joiner.join_along_y(line_size_z, p_x, shade);

                    joiner =
                    {
                        .p{ p_x },
                        .shade{ shade },
                        .type{ antialiasing_line_result_types::along_y }
                    };
                    joiner.p[0] = ~shade;
                    joiner.p[1] = shade;

                    _256_xf += _256_dxf;
                    p_y += d_p_y;
                }
                else
                {
                    joiner = invalid_result;
                }

                for (; p_y != end_p_y; p_y += d_p_y)
                {
                    if (_0_0 <= _256_xf) [[likely]]
                    {
                        if (_256_xf < _256_xf_aa_bound) [[likely]]
                        {
                            const auto index_shade = position_shade::instance_from_real(_256_xf);

                            joiner =
                            {
                                .p{ p_y + index_shade.index() },
                                .shade{ index_shade.shade() },
                                .type{ antialiasing_line_result_types::along_y  }
                            };
                            joiner.p[0] = ~joiner.shade;
                            joiner.p[1] = joiner.shade;
                        }
                        else
                        {
                            joiner = invalid_result;

                            const auto _256_xf_bound = _256_xf_aa_bound + _256_0;
                            if (_256_xf < _256_xf_bound)
                            {
                                const auto index_shade = position_shade::instance_from_real(_256_xf);
                                p_y[index_shade.index()] = ~index_shade.shade();
                            }
                        }
                    }
                    else
                    {
                        joiner = invalid_result;

                        if (_256_0_0_bound < _256_xf)
                        {
                            const auto index_shade = position_shade::instance_from_real(-_256_xf);
                            *p_y = ~index_shade.shade();
                        }
                    }

                    _256_xf += _256_dxf;
                }
            }
        }
        else
        {
            if (const auto [x0_z, x1_z] = round_range(direction_is_inc_x, image.width(), p0.x(), p1.x()); x0_z != x1_z) [[likely]]
            {
                const auto gradient_y = dy / dx;

                auto _256_yf = _256_0 * (p0.y() + gradient_y * (x0_z - p0.x()));
                const double _256_dyf = sign_if_not_likely(direction_is_inc_x, _256_0) * gradient_y;
                const double _256_yf_aa_bound = _256_0 * image.height() - _256_plus;

                auto p_x = image.data() + x0_z;
                const auto end_p_x = image.data() + x1_z;
                const auto d_p_x = sign_if_not_likely(direction_is_inc_x, 1_z);

                if (_0_0 <= _256_yf && _256_yf < _256_yf_aa_bound) [[likely]]
                {
                    const auto index_shade = position_shade::instance_from_real(_256_yf);
                    const auto p_y = p_x + index_shade.index() * line_size_z;
                    const auto shade = index_shade.shade();

                    joiner.join_along_x(line_size_z, p_y, shade);

                    joiner =
                    {
                        .p{ p_y },
                        .shade{ shade },
                        .type{ antialiasing_line_result_types::along_x }
                    };
                    joiner.p[0] = ~shade;
                    joiner.p[line_size_z] = shade;

                    _256_yf += _256_dyf;
                    p_x += d_p_x;
                }
                else
                {
                    joiner = invalid_result;
                }

                for (; p_x != end_p_x; p_x += d_p_x)
                {
                    if (_0_0 <= _256_yf) [[likely]]
                    {
                        if (_256_yf < _256_yf_aa_bound) [[likely]]
                        {
                            const auto index_shade = position_shade::instance_from_real(_256_yf);

                            joiner =
                            {
                                .p{ p_x + index_shade.index() * line_size_z },
                                .shade{ index_shade.shade() },
                                .type{ antialiasing_line_result_types::along_x  }
                            };
                            joiner.p[0] = ~joiner.shade;
                            joiner.p[line_size_z] = joiner.shade;
                        }
                        else
                        {
                            joiner = invalid_result;

                            const auto _256_yf_bound = _256_yf_aa_bound + _256_0;
                            if (_256_yf < _256_yf_bound)
                            {
                                const auto index_shade = position_shade::instance_from_real(_256_yf);
                                p_x[index_shade.index() * line_size_z] = ~index_shade.shade();
                            }
                        }
                    }
                    else
                    {
                        joiner = invalid_result;

                        if (_256_0_0_bound < _256_yf)
                        {
                            const auto index_shade = position_shade::instance_from_real(-_256_yf);
                            *p_x = ~index_shade.shade();
                        }
                    }

                    _256_yf += _256_dyf;
                }
            }
        }


        return joiner;
    }

    constexpr antialiasing_line_result draw_antialiasing_line
    (
        const lumpixspan image,
        const real_point2d p0,
        const real_point2d p1
    ) noexcept
    {
        return draw_antialiasing_line(image, p0, p1, invalid_antialiasing_line_result);
    }

    constexpr antialiasing_line_result draw_antialiasing_line
    (
        const lumpixspan image,
        const real_t x0,
        const real_t y0,
        const real_t x1,
        const real_t y1
    ) noexcept
    {
        return draw_antialiasing_line(image, point2d{ x0, y0 }, point2d{ x1, y1 });
    }

    template<class Transformation>
    constexpr void draw_polyline
    (
        const lumpixspan image,
        const real_point2d_cspan values,
        const Transformation value2px
    ) noexcept
    {
        if (values.size()) [[likely]]
        {
            auto cached_result = invalid_antialiasing_line_result;
            auto p0 = value2px(values.front());
            for (const auto& p : values.subspan(1u))
            {
                const auto p1 = value2px(p);
                {
                    const auto result = draw_antialiasing_line(image, p0, p1, cached_result);
                    cached_result = result;
                }
                p0 = p1;
            }
        }
    }


    namespace private_detail_algorithm
    {
        namespace private_detail_hv_line
        {
            constexpr pxvec line_width_range(real_t position, npx_t width, npx_t size) noexcept
            {
                D_ASSERT(is_positive(width));

                const auto half_width = _0_5 * width;
                const vec2 real_result{ position - half_width, position + half_width };
                const auto max_position = static_cast<real_t>(size);

                const vec2 result
                {
                    static_cast<npx_t>(std::clamp(real_result._0, _0_0, max_position)),
                    static_cast<npx_t>(std::clamp(real_result._1, _0_0, max_position))
                };

                D_ASSERT(result._1 >= result._0);
                return result;
            }
        }
    }

    void draw_hline
    (
        const lumpixspan image,
        real_t position,
        npx_t width
    ) noexcept
    {
        using namespace private_detail_algorithm::private_detail_hv_line;

        const auto range = line_width_range(position, width, image.height());
        const auto line_size = image.line_size();

        memset
        (
            image.data() + line_size * range._0,
            numeric_max_v<luminance_t>,
            line_size * (range._1 - range._0)
        );
    }

    constexpr void draw_vline
    (
        const lumpixspan image,
        real_t position,
        npx_t width
    ) noexcept
    {
        using namespace private_detail_algorithm::private_detail_hv_line;

        const auto range = line_width_range(position, width, image.width());
        const auto size = (range._1 - range._0);

        auto row_it = image.data() + range._0;
        const auto line_size = image.line_size();
        const auto row_it_end = row_it + line_size * image.height();

        for (; row_it != row_it_end; row_it += line_size)
        {
            auto it = row_it;
            const auto end_it = it + size;
            for (; it != end_it; ++it)
            {
                *it = numeric_max_v<luminance_t>;
            }
        }
    }

    D_WARNING_POP
}