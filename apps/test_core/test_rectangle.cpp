#include <core/rectangle.h>

#include <core/assert.h>

void test_rectangle() noexcept
{
    static_assert( std::is_trivial_v<rectangle<int>> && std::is_standard_layout_v<rectangle<int>> );

    constexpr point2d p0{1, 2};
    constexpr size2d sz{2u, 3u};
    constexpr auto p1 = p0 + narrow2d_cast<point2d<int>>(sz);

    constexpr rectangle rc{p0, sz};

    constexpr point2d p11_u{ p0.x() + rc.width(), p0.y() + rc.height() };
    constexpr auto p00_u = p1 - sz;

    static_assert(std::is_unsigned_v<decltype(p00_u.x())>);
    static_assert(std::is_unsigned_v<decltype(p11_u.x())>);
    static_assert(p11_u == (p0 + sz));
    static_assert(p11_u == (sz + p0));
    static_assert(p00_u == p11_u - sz);
    static_assert(sz + p00_u == p11_u);

    static_assert( rc.p00() == p0 );
    static_assert( rc.p01() == vec2{ p0.x(), p1.y() } );
    static_assert( rc.p10() == vec2{ p1.x(), p0.y() } );
    static_assert( rc.p11() == p1 );
    static_assert( rc.x0() == p0.x() );
    static_assert( rc.y0() == p0.y() );
    static_assert( rc.x1() == p1.x() );
    static_assert( rc.y1() == p1.y() );

    D_ASSERT( !errno );
}