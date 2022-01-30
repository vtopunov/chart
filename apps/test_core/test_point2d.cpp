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

    static_assert(narrow2d_cast<point2d<int>>(upt) == pt);
    static_assert(narrow2d_cast<vec2<int>>(upt) == v);
    static_assert(narrow2d_cast<point2d<unsigned>>(pt) == upt);
    static_assert(narrow2d_cast<vec2<unsigned>>(pt) == uv);

    constexpr point2d cpt{ '\5', '\6' };
    static_assert(std::is_same_v<decltype(cpt), const point2d<char>>);
    static_assert(cpt.x() == '\5');
    static_assert(cpt.y() == '\6');

    constexpr auto _2_cpt = '\2' * cpt;
    static_assert(std::is_same_v<decltype(_2_cpt), const point2d<decltype('\2' * cpt.x())> >);
    static_assert(_2_cpt.x() == '\2' * cpt.x());
    static_assert(_2_cpt.y() == '\2' * cpt.y());

    constexpr auto _2u_cpt = 2u * cpt;
    static_assert(std::is_same_v<decltype(_2u_cpt), const point2d<decltype(2u * cpt.x())> >);
    static_assert(_2u_cpt.x() == 2u * cpt.x());
    static_assert(_2u_cpt.y() == 2u * cpt.y());

    constexpr auto cpt_2 = cpt * '\2';
    static_assert(std::is_same_v<decltype(cpt_2), decltype(_2_cpt)>);
    static_assert(cpt_2 == _2_cpt);

    constexpr auto cpt_2u = cpt * 2u;
    static_assert(std::is_same_v<decltype(cpt_2u), decltype(_2u_cpt)>);
    static_assert(cpt_2u == _2u_cpt);



    D_ASSERT(!errno);
}