#pragma once

#include <core/warnings.h>

#include <px/pixspan.h>

#undef min
#undef max


namespace px
{
    constexpr bool draw_antialiasing_line(const pix8line pixs, double x0, double y0, double x1, double y1) noexcept
    {
        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_converting_from_floating_point_to_unsigned_integral);
        D_WARNING_DISABLE_MSVC(W_do_not_use_pointer_arithmetic);
        D_WARNING_DISABLE_MSVC(W_use_not_null);

        constexpr auto max_color = numeric_max_v<u8tint_t>;

        constexpr auto uz_round = [](double v) noexcept
        {
            return static_cast<size_t>(v + 0.5);
        };

        constexpr auto abs_distance = [](double v0, double v1) noexcept
        {
            return std::max(v0 - v1, v1 - v0);
        };

        constexpr auto _256_gradient = [](double dx, double dy) noexcept
        {
            constexpr auto eps = 0.5 / 256.0;
            return (eps < dx) ? (256.0 * (dy / dx)) : 256.0;
        };

        const auto swap_xy = abs_distance(x0, x1) < abs_distance(y0, y1);

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

        const auto _256_dy_by_dx = _256_gradient(x1 - x0, y1 - y0);

        auto _256_yf = 256.0 * (y0 + 0.5);

        if (swap_xy)
        {
            auto p = pixs.position + pixs.size * uz_round(x0);
            const auto end_p = pixs.position + pixs.size * (uz_round(x1) + 1u);

            for (; p != end_p; p += pixs.size)
            {
                {
                    const auto _256_y = static_cast<uint64_t>(_256_yf);
                    const size_t y = _256_y >> 8;
                    const uint8_t color = _256_y & max_color;

                    auto p_y = p + y;
                    *(p_y) = color;
                    *(--p_y) = max_color - color;
                }

                _256_yf += _256_dy_by_dx;
            }
        }
        else
        {
            auto p = pixs.position + uz_round(x0);
            const auto end_p = pixs.position + (uz_round(x1) + 1u);

            for (; p != end_p; ++p)
            {
                {
                    const auto _256_y = static_cast<uint64_t>(_256_yf);
                    const size_t y = _256_y >> 8;
                    const uint8_t color = _256_y & max_color;

                    auto p_y = p + y * pixs.size;
                    *(p_y) = color;
                    *(p_y -= pixs.size) = max_color - color;
                }

                _256_yf += _256_dy_by_dx;
            }
        }

        return true;

        D_WARNING_POP
    }
}