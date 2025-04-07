#include <chart/space_manipulation.h>

using chart::real_t;
using chart::real_point2d;
using chart::real_point2d_lowest_inf;
using chart::space_diagonal;
using chart::space_diagonal_initializer;


void test_space_manipulation() noexcept
{
    constexpr auto real_point2d_lowest = fill_to<point2d>(numeric_lowest_v<real_t>);
    constexpr auto real_point2d_max = fill_to<point2d>(numeric_max_v<real_t>);

    static_assert(real_point2d_lowest == md_min(space_diagonal_initializer._0, real_point2d_lowest));
    static_assert(real_point2d_max == md_min(space_diagonal_initializer._0, real_point2d_max));
    static_assert(real_point2d_lowest == md_min(md_min(space_diagonal_initializer._0, real_point2d_lowest), real_point2d_max));
    static_assert(real_point2d_lowest == md_min(md_min(space_diagonal_initializer._0, real_point2d_max), real_point2d_lowest));

    static_assert(real_point2d_lowest == md_max(space_diagonal_initializer._1, real_point2d_lowest));
    static_assert(real_point2d_max == md_max(space_diagonal_initializer._1, real_point2d_max));
    static_assert(real_point2d_max == md_max(md_max(space_diagonal_initializer._1, real_point2d_lowest), real_point2d_max));
    static_assert(real_point2d_max == md_max(md_max(space_diagonal_initializer._1, real_point2d_max), real_point2d_lowest));

    D_ASSERT(!errno);
}