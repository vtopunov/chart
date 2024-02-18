#include <core/rectangle.h>


void test_rectangle() noexcept
{
    static_assert(std::is_trivial_v<rectangle<int>> && std::is_standard_layout_v<rectangle<int>>);

    constexpr point2d p0{ 1, 2 };
    constexpr size2d sz{ 2u, 3u };
    
    constexpr point2d up0
    {
        to_unsigned(p0.x()),
        to_unsigned(p0.y())
    };

    static_assert((p0 + sz) == point2d{ 3u, 5u });
    static_assert((sz + p0) == point2d{ 3u, 5u });
    static_assert((p0 * sz) == point2d{ 2u, 6u });
    static_assert((sz * p0) == point2d{ 2u, 6u });

    constexpr auto p1 = p0 + md_narrow<size2d<int>>(sz);
    static_assert(p1 == (p0 + md_narrow<point2d<int>>(sz)));
    static_assert((p1 - sz) == up0);
    static_assert((sz - p0) == point2d{1u, 1u});
    static_assert((point2d{1.0, 2.0} / sz) == point2d{ 1.0 / sz.width(), 2.0 / sz.height()});


    constexpr rectangle rc{ p0, sz };

    constexpr point2d p11_u{ p0.x() + rc.width(), p0.y() + rc.height() };
    constexpr auto p00_u = p1 - sz;

    static_assert(std::is_unsigned_v<decltype(p00_u.x())>);
    static_assert(std::is_unsigned_v<decltype(p11_u.x())>);
    static_assert(p11_u == (p0 + sz));
    static_assert(p11_u == (sz + p0));
    static_assert(p00_u == p11_u - sz);
    static_assert(sz + p00_u == p11_u);

    static_assert(rc.p00() == p0);
    static_assert(rc.p01() == point2d{ p0.x(), p1.y() });
    static_assert(rc.p10() == point2d{ p1.x(), p0.y() });
    static_assert(rc.p11() == p1);
    static_assert(rc.x0() == p0.x());
    static_assert(rc.y0() == p0.y());
    static_assert(rc.x1() == p1.x());
    static_assert(rc.y1() == p1.y());
    static_assert(rc.sizes == sizes(rc));
    static_assert(rc.width() == width(rc));
    static_assert(rc.height() == height(rc));

    D_ASSERT(!errno);
}