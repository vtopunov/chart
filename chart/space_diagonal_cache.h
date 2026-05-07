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

        struct diagonal_lenght_range_factory
        {
            const pxsizes sizes;
            
            [[nodiscard]] 
            constexpr point2re min() const noexcept
            {
                return { sizes * numeric_eps_v<real_t> };
            }

            [[nodiscard]]
            constexpr point2re max() const noexcept
            {
                constexpr auto real_sz_max = fill_to<point2re>(numeric_max_v<>);
                D_ASSERT(md_is_positiven(sizes));
                return real_sz_max / sizes;
            }

            [[nodiscard]]
            constexpr point2re max(const point2re& lenght0) const noexcept
            {
                return lenght0 * (0.5 * sizes);
            }

            template<class... Args>
            [[nodiscard]] constexpr vec2<point2re> make(const Args&... args) const noexcept
            {
                return { min(), max(args...) };
            }
        };

        template<class... Args>
        [[nodiscard]] constexpr bool diagonal_lenght_inrange(const point2re& lenght, const pxsizes pxsizes, const Args&... args) noexcept
        {
            const diagonal_lenght_range_factory range_factory{ pxsizes };
            return inrange_neqfp(lenght, range_factory.make(args...));
        }
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
        constexpr bool try_update(const space_diagonal& line, const pxsizes pxsizes) noexcept
        {
            using private_detail_space_diagonal_cache::diagonal_lenght_inrange;
            D_ASSERT(has_value());

            if (const auto dline = line._1 - line._0; md_isnormal(dline))
            {
                if (diagonal_lenght_inrange(dline, pxsizes, dline0_))
                {
                    line_ = line;
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]]
        constexpr bool try_first_update(const space_diagonal& line, const pxsizes pxsizes) noexcept
        {
            using private_detail_space_diagonal_cache::diagonal_lenght_inrange;
            D_ASSERT(!has_value());

            if (const auto dline = line._1 - line._0; md_isnormal(dline))
            {
                if (diagonal_lenght_inrange(dline, pxsizes))
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

        [[nodiscard]]
        static constexpr bool dline_has_value(const point2re& pt) noexcept
        {
            const auto has = has_dvalue(pt.y());
            D_ASSERT(has == has_dvalue(pt.x()));
            return has;
        }

        [[nodiscard]]
        static constexpr bool has_dvalue(real_t dvalue) noexcept
        {
            const auto has = (invalid_dvalue != dvalue);
            D_ASSERT(!has || std::isnormal(dvalue));
            return has;
        }

    private:
        space_diagonal line_{ chart::space_diagonal_initializer };
        point2re dline0_{ invalid_dline };
    };
}