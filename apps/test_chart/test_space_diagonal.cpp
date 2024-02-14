#include <chart/space_diagonal.h>

using chart::real_t;
using chart::real_point2d;
using chart::real_point2d_lowest_inf;
using chart::space_diagonal_t;
using chart::space_diagonal_initializer;
using chart::space_diagonal_with;

namespace
{
    constexpr auto real_point2d_lowest = fill_to<point2d>(numeric_lowest_v<real_t>);
    constexpr auto real_point2d_max = fill_to<point2d>(numeric_max_v<real_t>);
}


void test_polyline_space_diagonal() noexcept
{
    static_assert(real_point2d_lowest == md_min(space_diagonal_initializer._0, real_point2d_lowest));
    static_assert(real_point2d_max == md_min(space_diagonal_initializer._0, real_point2d_max));
    static_assert(real_point2d_lowest == md_min(md_min(space_diagonal_initializer._0, real_point2d_lowest), real_point2d_max));
    static_assert(real_point2d_lowest == md_min(md_min(space_diagonal_initializer._0, real_point2d_max), real_point2d_lowest));

    static_assert(real_point2d_lowest == md_max(space_diagonal_initializer._1, real_point2d_lowest));
    static_assert(real_point2d_max == md_max(space_diagonal_initializer._1, real_point2d_max));
    static_assert(real_point2d_max == md_max(md_max(space_diagonal_initializer._1, real_point2d_lowest), real_point2d_max));
    static_assert(real_point2d_max == md_max(md_max(space_diagonal_initializer._1, real_point2d_max), real_point2d_lowest));

    {
        using chart::private_detail_space_diagonal::is_great_neq;

        {
            const struct errno_restorer
            {
                int errvalue;

                errno_restorer() noexcept
                    : errvalue{ errno }
                {}

                ~errno_restorer() noexcept
                {
                    errno = errvalue;
                }
            } hold_errno;

            D_ASSERT(!is_great_neq(0.0, 0.0));
            D_ASSERT(!is_great_neq(u_next(0.0), 0.0));
            D_ASSERT(is_great_neq(u_next(u_next(0.0)), 0.0));
        }

        {
            constexpr auto nearz = 3 * numeric_eps_v<real_t>;
            D_ASSERT(!is_great_neq(nearz, nearz));
            D_ASSERT(!is_great_neq(u_next(nearz), nearz));
            D_ASSERT(is_great_neq(u_next(u_next(nearz)), nearz));
        }
    }

    D_ASSERT(!errno);
}