#include<core/point2d.h>

void test_point2d() noexcept
{
    constexpr vec2 v{ 3, 4 };
    constexpr point2d pt{ v };
    static_assert(std::is_same_v<decltype(pt), const point2d<int>>);
    static_assert(std::is_trivial_v<point2d<int>> && std::is_standard_layout_v<point2d<int>>);
    static_assert(pt.x() == 3);
    static_assert(pt.y() == 4);

    constexpr vec2 uv{ 3u, 4u };
    constexpr point2d upt{ uv };
    static_assert(std::is_same_v<decltype(upt), const point2d<unsigned>>);

    static_assert(vec2_cast<point2d<int>>(upt) == pt);
    static_assert(vec2_cast<vec2<int>>(upt) == v);
    static_assert(vec2_cast<point2d<unsigned>>(pt) == upt);
    static_assert(vec2_cast<vec2<unsigned>>(pt) == uv);

    constexpr point2d cpt{ '\5', '\6' };
    static_assert(std::is_same_v<decltype(cpt), const point2d<char>>);
    static_assert(cpt.x() == '\5');
    static_assert(cpt.y() == '\6');

    D_ASSERT(!errno);
}