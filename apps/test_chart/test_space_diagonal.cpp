#include <chart/space_diagonal.h>

using chart::real_t;
using chart::real_point2d;
using chart::real_point2d_lowest_inf;
using chart::space_diagonal_t;
using chart::space_diagonal_value_invalid_mark;
using chart::space_diagonal_initializer;
using chart::space_diagonal_has_invalid_mark;
using chart::space_diagonal_with;

namespace
{
    constexpr auto real_point2d_lowest = fill_to<point2d>(numeric_lowest_v<real_t>);
    constexpr auto real_point2d_max = fill_to<point2d>(numeric_max_v<real_t>);
    constexpr auto space_diagonal_point2d_invalid_mark = fill_to<point2d>(space_diagonal_value_invalid_mark);
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

    static_assert(space_diagonal_has_invalid_mark(space_diagonal_initializer));
    static_assert(!space_diagonal_has_invalid_mark(inverse(space_diagonal_initializer)));

    static_assert(space_diagonal_has_invalid_mark(space_diagonal_with(space_diagonal_initializer, space_diagonal_point2d_invalid_mark)));
    static_assert(!space_diagonal_has_invalid_mark(space_diagonal_with(space_diagonal_initializer, real_point2d_lowest)));
    static_assert(!space_diagonal_has_invalid_mark(space_diagonal_with(space_diagonal_initializer, real_point2d_max)));

    {
        auto space = space_diagonal_initializer;
        D_ASSERT(space_diagonal_has_invalid_mark(space));
        D_ASSERT(space_diagonal_value_invalid_mark == space._0._0);
        D_ASSERT(space_diagonal_point2d_invalid_mark == space._0);

        space._0._0 = std::nextafter(space._0._0, 0.0);
        D_ASSERT(!space_diagonal_has_invalid_mark(space));
        D_ASSERT(!(space_diagonal_value_invalid_mark == space._0._0));
        D_ASSERT(!(space_diagonal_point2d_invalid_mark == space._0));

        space = space_diagonal_with(space, space_diagonal_point2d_invalid_mark);
        D_ASSERT(!space_diagonal_has_invalid_mark(space));
        D_ASSERT(space_diagonal_point2d_invalid_mark != space._0);
        D_ASSERT(space_diagonal_point2d_invalid_mark == space._1);
    }

    D_ASSERT(!errno);
}