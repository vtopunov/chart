#include <chart/space_manipulation.h>


using chart::space_diagonal;
using chart::space_diagonal_initializer;


void test_space_manipulation() noexcept
{
    constexpr auto point2re_lowest = fill_to<point2re>(numeric_lowest_v<>);
    constexpr auto point2re_max = fill_to<point2re>(numeric_max_v<>);

    static_assert(point2re_lowest == md_min(space_diagonal_initializer._0, point2re_lowest));
    static_assert(point2re_max == md_min(space_diagonal_initializer._0, point2re_max));
    static_assert(point2re_lowest == md_min(md_min(space_diagonal_initializer._0, point2re_lowest), point2re_max));
    static_assert(point2re_lowest == md_min(md_min(space_diagonal_initializer._0, point2re_max), point2re_lowest));

    static_assert(point2re_lowest == md_max(space_diagonal_initializer._1, point2re_lowest));
    static_assert(point2re_max == md_max(space_diagonal_initializer._1, point2re_max));
    static_assert(point2re_max == md_max(md_max(space_diagonal_initializer._1, point2re_lowest), point2re_max));
    static_assert(point2re_max == md_max(md_max(space_diagonal_initializer._1, point2re_max), point2re_lowest));

    D_ASSERT(!errno);
}