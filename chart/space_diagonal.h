#pragma once

#include <core/point2d.h>

#include <px/fwd.h>


namespace chart
{
    using px::real_t;
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

    constexpr auto space_diagonal_value_invalid_mark = space_diagonal_initializer._0._0;

    [[nodiscard]]
    constexpr bool space_diagonal_has_invalid_mark(real_t value) noexcept
    {
        return space_diagonal_value_invalid_mark == value;
    }

    template<class T>
    [[nodiscard]] constexpr bool space_diagonal_has_invalid_mark(const vec2<T>& v) noexcept
    {
        return space_diagonal_has_invalid_mark(v._0);
    }

    static_assert(space_diagonal_has_invalid_mark(space_diagonal_initializer._0));
    static_assert(space_diagonal_has_invalid_mark(space_diagonal_initializer));


    namespace private_detail_space_diagonal
    {
        template<class L, class R>
        [[nodiscard]] constexpr auto md_each_less(const L& a, const R& b) noexcept -> decltype(a < b)
        {
            return a < b;
        }

        template<class L, class R>
        [[nodiscard]] constexpr auto md_each_less(const L& a, const R& b) noexcept -> decltype(md_each_less(as_vec2(a)._0, as_vec2(b)._0))
        {
            return md_each_less(a._0, b._0)
                && md_each_less(a._1, b._1);
        }
    }

    [[nodiscard]]
    constexpr bool space_diagonal_is_good(const space_diagonal_t& line) noexcept
    {
        using namespace private_detail_space_diagonal;

        return md_each_less(line._0, line._1)
            && md_isfinite(line)
            && md_isnormal(line._1 - line._0);
    }

    static_assert(!space_diagonal_is_good(space_diagonal_initializer));


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
    public:
        constexpr space_diagonal_cache() noexcept = default;
        D_DISABLE_COPY_MOVE(space_diagonal_cache);

        constexpr explicit operator bool() const noexcept
        {
            return has_value();
        }

        constexpr void fore_update(const space_diagonal_t& line) noexcept
        {
            D_ASSERT(space_diagonal_is_good(line));
            line_ = line;
        }

        [[nodiscard]]
        constexpr bool try_update(const space_diagonal_t& line) noexcept
        {
            if (chart::space_diagonal_is_good(line)) [[likely]]
            {
                line_ = line;
                return true;
            }

            return false;
        }

        [[nodiscard]]
        constexpr bool has_value() const noexcept
        {
            return !space_diagonal_has_invalid_mark(line_);
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
        }

    private:
        space_diagonal_t line_{ chart::space_diagonal_initializer };
    };
}