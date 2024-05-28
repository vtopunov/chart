#include <chart/space_diagonal_cache.h>

using chart::real_t;


namespace
{
    template<class T>
    [[nodiscard]] constexpr bool is_great_neq(T value, T max_value) noexcept
    {
        return chart::private_detail_space_diagonal_cache::is_less_neq(max_value, value);
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_floating_point_v<T>, T> prevf(T value) noexcept
    {
        return std::nextafter(value, -std::numeric_limits<T>::infinity());
    }
}

void test_space_diagonal_cache() noexcept
{
    {
        using chart::private_detail_space_diagonal_cache::nextf;

        {
            const auto nfz = nextf(0.0);
            D_ASSERT(nfz > 0.0 && nfz <= DBL_EPSILON && !std::isnormal(nfz));
        }

        {
            D_ASSERT(!is_great_neq(0.0, 0.0));
            D_ASSERT(!is_great_neq(nextf(0.0), 0.0));
            D_ASSERT(is_great_neq(nextf(nextf(0.0)), 0.0));
        }

        {
            constexpr auto nearz = 3 * numeric_eps_v<real_t>;
            D_ASSERT(!is_great_neq(nearz, nearz));
            D_ASSERT(!is_great_neq(nextf(nearz), nearz));
            D_ASSERT(is_great_neq(nextf(nextf(nearz)), nearz));
        }
    }

    {
        using chart::private_detail_space_diagonal_cache::is_less_neq;

        {

            const auto pfz = prevf(0.0);
            D_ASSERT(pfz < 0.0 && pfz >= -DBL_EPSILON && !std::isnormal(pfz));
        }

        {
            D_ASSERT(!is_less_neq(0.0, 0.0));
            D_ASSERT(!is_less_neq(prevf(0.0), 0.0));
            D_ASSERT(is_less_neq(prevf(prevf(0.0)), 0.0));
        }

        {
            constexpr auto nearz = 3 * numeric_eps_v<real_t>;
            D_ASSERT(!is_less_neq(nearz, nearz));
            D_ASSERT(!is_less_neq(prevf(nearz), nearz));
            D_ASSERT(is_less_neq(prevf(prevf(nearz)), nearz));
        }
    }

    {
        using chart::private_detail_space_diagonal_cache::inrange_neq;
        using chart::private_detail_space_diagonal_cache::nextf;

        D_ASSERT(!inrange_neq(0.0, 0.0, 0.0));
        D_ASSERT(!inrange_neq(0.0, prevf(0.0), 0.0));
        D_ASSERT(!inrange_neq(0.0, 0.0, nextf(0.0)));
        D_ASSERT(!inrange_neq(0.0, prevf(0.0), nextf(0.0)));
        D_ASSERT(!inrange_neq(0.0, prevf(prevf(0.0)), nextf(0.0)));
        D_ASSERT(!inrange_neq(0.0, prevf(0.0), nextf(nextf(0.0))));
        D_ASSERT(inrange_neq(0.0, prevf(prevf(0.0)), nextf(nextf(0.0))));
    }
}