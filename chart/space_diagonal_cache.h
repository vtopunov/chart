#pragma once

#include <chart/space_manipulation.h>


namespace chart
{
    namespace private_detail_space_diagonal_cache
    {
        template<class T>
        [[nodiscard]] std::enable_if_t<std::is_floating_point_v<T>, T> nextf(T value) noexcept
        {
            return std::nextafter(value, std::numeric_limits<T>::infinity());
        }

        template<class T>
        [[nodiscard]] constexpr auto is_less_neq(T value, T max_value) noexcept -> decltype(nextf(value) < max_value)
        {
            return (value < max_value) && (nextf(value) < max_value);
        }

        template<class T>
        [[nodiscard]] constexpr auto inrange_neq(T value, T min_value, T max_value) noexcept -> decltype(is_less_neq(min_value, value))
        {
            return is_less_neq(min_value, value)
                && is_less_neq(value, max_value);
        }

        template<class T>
        [[nodiscard]] constexpr auto md_inrange_neq(const T& value, const T& min_value, const T& max_value) noexcept -> decltype(inrange_neq(value, min_value, max_value))
        {
            return inrange_neq(value, min_value, max_value);
        }


        template<class T>
        [[nodiscard]] constexpr auto md_inrange_neq(const T& value, const T& min_value, const T& max_value) noexcept
            -> decltype(md_inrange_neq(as_vec2(value)._0, min_value._0, max_value._0))
        {
            return md_inrange_neq(value._0, min_value._0, max_value._0)
                && md_inrange_neq(value._1, min_value._1, max_value._1);
        }

        template<class T>
        [[nodiscard]] constexpr bool md_inrange_neq(const T& value, const vec2<T>& range) noexcept
        {
            return md_inrange_neq(value, range._0, range._1);
        }

        [[nodiscard]] constexpr real_point2d calculate_dline_min(pxsize2d pxsizes) noexcept
        {
            return { pxsizes * numeric_eps_v<real_t> };
        }

        [[nodiscard]] constexpr vec2<real_point2d> calculate_dline_range(real_point2d dline0, pxsize2d pxsizes) noexcept
        {
            return
            {
                calculate_dline_min(pxsizes),
                dline0 * (0.5 * pxsizes)
            };
        }

        [[nodiscard]] constexpr vec2<real_point2d> calculate_dline0_range(pxsize2d pxsizes) noexcept
        {
            constexpr auto real_sz_max = fill_to<size2d>(numeric_max_v<real_t>);
            D_ASSERT(pxsizes.has_positive_square());
            return
            {
                calculate_dline_min(pxsizes),
                real_sz_max / pxsizes
            };
        }
    }

    class space_diagonal_cache
    {
    public:
        constexpr explicit operator bool() const noexcept
        {
            return has_value();
        }

        [[nodiscard]]
        constexpr bool try_update(const space_diagonal& line, pxsize2d pxsizes) noexcept
        {
            using private_detail_space_diagonal_cache::md_inrange_neq;
            using private_detail_space_diagonal_cache::calculate_dline_range;
            D_ASSERT(dline_has_value(dline0_));

            if (const auto dline = line._1 - line._0; md_isnormal(dline))
            {
                if (md_inrange_neq(dline, calculate_dline_range(dline0_, pxsizes)))
                {
                    line_ = line;
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]]
        constexpr bool try_first_update(const space_diagonal& line, pxsize2d pxsizes) noexcept
        {
            using private_detail_space_diagonal_cache::md_inrange_neq;
            using private_detail_space_diagonal_cache::calculate_dline0_range;
            D_ASSERT(!dline_has_value(dline0_));

            if (const auto dline = line._1 - line._0; md_isnormal(dline))
            {
                if (md_inrange_neq(dline, calculate_dline0_range(pxsizes)))
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
        static constexpr bool dline_has_value(const real_point2d& pt) noexcept
        {
            return invalid_dvalue != pt.y();
        }

    private:
        space_diagonal line_{ chart::space_diagonal_initializer };
        real_point2d dline0_{ invalid_dline };
    };
}