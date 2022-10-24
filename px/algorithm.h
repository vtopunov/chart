#pragma once

#include <core/warnings.h>

#include <px/pixspan.h>

#undef min
#undef max


namespace px
{
    constexpr void draw_antialiasing_line(const pix8span image, double x0, double y0, double x1, double y1) noexcept
    {
        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_converting_from_floating_point_to_unsigned_integral);
        D_WARNING_DISABLE_MSVC(W_do_not_use_pointer_arithmetic);
        D_WARNING_DISABLE_MSVC(W_use_not_null);

        constexpr auto max_color = numeric_max_v<u8tint_t>;

        constexpr auto eps = 0.5 / 256.0;

        constexpr auto uz_round = [] (double v) noexcept
        {
            return static_cast<size_t>(v + 0.5);
        };

        constexpr auto _256_gradient = [] (double dx, double dy) noexcept
        {
            return (eps < dx) ? (256.0 * (dy / dx)) : 256.0;
        };


        const auto swap_xy = constexpr_abs(x1 - x0) < constexpr_abs(y1 - y0);

        if (swap_xy)
        {
            std::swap(x0, y0);
            std::swap(x1, y1);
        }

        if (x1 < x0)
        {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        if (x1 < 0.0)
        {
            return;
        }

        const auto line_size = image.line_size();

        const auto _256_dy_by_dx = _256_gradient(x1 - x0, y1 - y0);

        auto _256_yf = 256.0 * (y0 + 0.5);

        if (x0 < 0.0)
        {
            _256_yf -= _256_dy_by_dx * x0;
            x0 = 0.0;
        }

        if (swap_xy)
        {
            {
                const auto xf_bound = image.height() - (0.5 + eps);

                if (x0 > xf_bound)
                {
                    return;
                }

                if (x1 > xf_bound)
                {
                    x1 = xf_bound;
                }
            }

            auto p = image.data() + line_size * uz_round(x0);
            const auto end_p = image.data() + line_size * (uz_round(x1) + 1_uz);
            const auto _256_yf_size = 256.0 * image.width();

            for (; p != end_p; p += line_size)
            {
                if (D_LIKELY(_256_yf >= 0.0 && _256_yf < _256_yf_size)) D_ATTRIB_LIKELY
                {
                    const auto _256_y = static_cast<uint64_t>(_256_yf);
                    const size_t y = _256_y >> 8;
                    const uint8_t color = _256_y & max_color;

                    auto p_y = p + y;
                    *(p_y) = color;

                    if (D_LIKELY(y)) D_ATTRIB_LIKELY
                    {
                        *(--p_y) = max_color - color;
                    }
                }

                _256_yf += _256_dy_by_dx;
            }
        }
        else
        {
            {
                const auto xf_bound = image.width() - (0.5 + eps);

                if (x0 > xf_bound)
                {
                    return;
                }

                if (x1 > xf_bound)
                {
                    x1 = xf_bound;
                }
            }

            auto p = image.data() + uz_round(x0);
            const auto end_p = image.data() + (uz_round(x1) + 1_uz);
            const double _256_yf_size = 256.0 * image.height();

            for (; p != end_p; ++p)
            {
                if (D_LIKELY(_256_yf >= 0.0 && _256_yf < _256_yf_size)) D_ATTRIB_LIKELY
                {
                    const auto _256_y = static_cast<uint64_t>(_256_yf);
                    const size_t y = _256_y >> 8;
                    const uint8_t color = _256_y & max_color;

                    auto p_y = p + y * line_size;
                    *(p_y) = color;

                    if (D_LIKELY(y)) D_ATTRIB_LIKELY
                    {
                        *(p_y -= line_size) = max_color - color;
                    }
                }

                _256_yf += _256_dy_by_dx;
            }
        }

        D_WARNING_POP
    }
}