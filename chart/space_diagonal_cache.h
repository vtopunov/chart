#pragma once

#include <chart/space_manipulation.h>


namespace chart
{
    namespace private_detail_space_diagonal_cache
    {
        [[nodiscard]] constexpr auto inrange_neqfp(real_t value, real_t min_value, real_t max_value) noexcept
        {
            return is_less_neqfp(min_value, value)
                && is_less_neqfp(value, max_value);
        }

        [[nodiscard]] constexpr auto inrange_neqfp(const point2re& pt, const vec2<point2re>& range) noexcept
        {
            return inrange_neqfp(pt.x(), range._0.x(), range._1.x())
                && inrange_neqfp(pt.y(), range._0.y(), range._1.y());
        }

        struct diagonal_lenght_range
        {
            const pxsizes sizes;
            
            [[nodiscard]] 
            constexpr point2re min() const noexcept
            {
                return { sizes * numeric_eps_v<real_t> };
            }

            [[nodiscard]]
            constexpr point2re max0() const noexcept
            {
                constexpr auto real_sz_max = fill_to<point2re>(numeric_max_v<>);
                D_ASSERT_OR_ASSUME(sizes.has_positive_square());
                return real_sz_max / sizes;
            }

            [[nodiscard]]
            constexpr point2re max(const point2re& lenght0) const noexcept
            {
                return lenght0 * (0.5 * sizes);
            }

            [[nodiscard]] 
            constexpr vec2<point2re> first() const noexcept
            {
                return { min(), max0() };
            }

            [[nodiscard]] 
            constexpr vec2<point2re> for_update(const point2re& lenght0) const noexcept
            {
                return { min(), max(lenght0) };
            }
        };
    }

    class space_diagonal_cache
    {
    public:
        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return has_value();
        }

        [[nodiscard]]
        constexpr bool try_update(const space_diagonal& line, pxsizes pxsizes) noexcept
        {
            using private_detail_space_diagonal_cache::inrange_neqfp;
            using private_detail_space_diagonal_cache::diagonal_lenght_range;
            D_ASSERT_OR_ASSUME(has_value());

            if (const auto dline = line._1 - line._0; md_isnormal(dline))
            {
                if (inrange_neqfp(dline, diagonal_lenght_range(pxsizes).for_update(dline0_)))
                {
                    line_ = line;
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]]
        constexpr bool try_first_update(const space_diagonal& line, pxsizes pxsizes) noexcept
        {
            using private_detail_space_diagonal_cache::inrange_neqfp;
            using private_detail_space_diagonal_cache::diagonal_lenght_range;
            D_ASSERT_OR_ASSUME(!has_value());

            if (const auto dline = line._1 - line._0; md_isnormal(dline))
            {
                if (inrange_neqfp(dline, diagonal_lenght_range(pxsizes).first()))
                {
                    line_ = line;
                    dline0_ = dline;
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
        constexpr space_diagonal value() const noexcept
        {
            D_ASSERT_OR_ASSUME(has_value());
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

        [[nodiscard]]
        static constexpr bool dline_has_value(const point2re& pt) noexcept
        {
            const auto has = has_dvalue(pt.y());
            D_ASSERT_OR_ASSUME(has == has_dvalue(pt.x()));
            return has;
        }

        [[nodiscard]]
        static constexpr bool has_dvalue(real_t dvalue) noexcept
        {
            const auto has = (invalid_dvalue != dvalue);
            D_ASSERT_OR_ASSUME(!has || std::isnormal(dvalue));
            return has;
        }

    private:
        space_diagonal line_{ chart::space_diagonal_initializer };
        point2re dline0_{ invalid_dline };
    };
}