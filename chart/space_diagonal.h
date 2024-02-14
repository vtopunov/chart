#pragma once

#include <core/point2d.h>

#include <px/fwd.h>


namespace chart
{
    using px::real_t;
    using px::real_vec2;
    using px::real_point2d;
    using px::real_point2d_cspan;

    constexpr auto real_inf = numeric_inf_v<real_t>;
    constexpr auto real_lowest_inf = -real_inf;
    constexpr auto real_point2d_inf = fill_to<point2d>(real_inf);
    constexpr auto real_point2d_lowest_inf = fill_to<point2d>(real_lowest_inf);

    using space_diagonal_t = vec2<real_point2d>;

    constexpr space_diagonal_t space_diagonal_initializer
    {
        real_point2d_inf,
        real_point2d_lowest_inf,
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

        [[nodiscard]] inline bool md_inrange_neq(const real_vec2& value, const real_vec2& min_value,  const real_vec2& max_value) noexcept
        {
            return inrange_neq(value._0, min_value._0, max_value._0)
                && inrange_neq(value._1, min_value._1, max_value._1);
        }
    }

    [[nodiscard]]
    constexpr space_diagonal_t space_diagonal_with(const space_diagonal_t& dia, const real_point2d& pt) noexcept
    {
        return
        {
            md_min(dia._0, pt),
            md_max(dia._1, pt)
        };
    }

    [[nodiscard]]
    constexpr space_diagonal_t space_diagonal_with(space_diagonal_t diagonal, real_point2d_cspan line) noexcept
    {
        for (const auto& pt : line) [[likely]]
        {
            if (md_isfinite(pt)) [[likely]]
            {
                diagonal = space_diagonal_with(diagonal, pt);
            }
        }

        return diagonal;
    }


    [[nodiscard]]
    constexpr space_diagonal_t make_pix_space_diagonal(pxsize2d sizes) noexcept
    {
        constexpr auto real_zero = zero_v<real_t>;

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
        static constexpr real_t invalid_dvalue{ 0.0 };
        static constexpr auto invalid_dline = fill_to<point2d>(invalid_dvalue);
        
        static constexpr bool dline_has_value(const real_point2d& pt) noexcept
        {
            return invalid_dvalue != pt.y();
        }

    public:
        constexpr space_diagonal_cache() noexcept = default;
        D_DISABLE_COPY_MOVE(space_diagonal_cache);

        constexpr explicit operator bool() const noexcept
        {
            return has_value();
        }

        [[nodiscard]]
        constexpr bool try_update(const space_diagonal_t& line, pxsize2d pxsizes) noexcept
        {
            using private_detail_space_diagonal::md_inrange_neq;

            const auto dline = line._1 - line._0;
            if(md_isnormal(dline))
            {
                constexpr auto num_max = fill_to<point2d>(numeric_max_v<real_t>);
                const auto has_dline = dline_has_value(dline0_);
                const auto min_dline = pxsizes * numeric_eps_v<real_t>;
                const auto max_dline = (has_dline) ? 0.5 * (pxsizes * dline0_) : num_max;
                if(md_inrange_neq(dline, min_dline, max_dline))
                {
                    if(!has_dline)
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
        space_diagonal_t line_{ chart::space_diagonal_initializer };
        real_point2d dline0_{ invalid_dline };
    };
}