#pragma once

#include <chart/fwd.h>


namespace chart
{
    constexpr auto real_inf = numeric_inf_v<real_t>;
    constexpr auto real_lowest_inf = -real_inf;
    constexpr auto real_point2d_inf = fill_to<point2d>(real_inf);
    constexpr auto real_point2d_lowest_inf = fill_to<point2d>(real_lowest_inf);

    constexpr space_diagonal_t space_diagonal_initializer
    {
        real_point2d_inf,
        real_point2d_lowest_inf
    };

    namespace private_detail_space_diagonal
    {
        [[nodiscard]] inline bool is_great_neq(real_t  value, real_t min_value) noexcept
        {
            return min_value < u_prev(value);
        }

        [[nodiscard]] inline bool is_less_neq(real_t value, real_t max_value) noexcept
        {
            return u_next(value) < max_value;
        }

        [[nodiscard]] inline bool inrange_neq(real_t value, real_t min_value, real_t max_value) noexcept
        {
            return is_great_neq(value, min_value)
                && is_less_neq(value, max_value);
        }

        [[nodiscard]] inline bool md_inrange_neq(const real_vec2& value, const real_vec2& min_value, const real_vec2& max_value) noexcept
        {
            return inrange_neq(value._0, min_value._0, max_value._0)
                && inrange_neq(value._1, min_value._1, max_value._1);
        }
    }

    [[nodiscard]]
    constexpr space_diagonal_t make_pix_space_diagonal(pxsize2d sizes) noexcept
    {
        constexpr real_t real_zero{ zero_v<> };

        D_ASSERT(sizes.width());
        D_ASSERT(sizes.height());

        const auto [x, y] = md_narrow<real_point2d>(sizes);

        return
        {
            { real_zero, y },
            { x, real_zero },
        };
    }

    class space_diagonal_cache
    {
    public:
        constexpr explicit operator bool() const noexcept
        {
            return has_value();
        }

        [[nodiscard]]
        constexpr bool try_update(const space_diagonal_t& line, pxsize2d pxsizes) noexcept
        {
            using private_detail_space_diagonal::md_inrange_neq;

            const auto dline = line._1 - line._0;
            if (md_isnormal(dline))
            {
                constexpr auto num_max = fill_to<point2d>(numeric_max_v<real_t>);
                const auto has_dline = dline_has_value(dline0_);
                const auto min_dline = pxsizes * numeric_eps_v<real_t>;
                const auto max_dline = (has_dline) ? 0.5 * (pxsizes * dline0_) : num_max;
                if (md_inrange_neq(dline, min_dline, max_dline))
                {
                    if (!has_dline)
                    {
                        dline0_ = dline;
                    }

                    line_ = line;
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]]
        constexpr bool has_value() const noexcept
        {
            return dline_has_value(dline0_);
        }

        [[nodiscard]]
        constexpr space_diagonal_t value() const noexcept
        {
            D_ASSERT(has_value());
            return line_;
        }

        constexpr void clear() noexcept
        {
            line_ = chart::space_diagonal_initializer;
            dline0_ = invalid_dline;
        }

    private:
        static constexpr real_t invalid_dvalue{ 0.0 };
        static constexpr auto invalid_dline = fill_to<point2d>(invalid_dvalue);

        static constexpr bool dline_has_value(const real_point2d& pt) noexcept
        {
            return invalid_dvalue != pt.y();
        }

    private:
        space_diagonal_t line_{ chart::space_diagonal_initializer };
        real_point2d dline0_{ invalid_dline };
    };
}