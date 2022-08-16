#include <core/rectangle.h>

#include <core/assert.h>

void test_rectangle() noexcept
{
    static_assert( std::is_trivial_v<rectangle<int>> && std::is_standard_layout_v<rectangle<int>> );

    constexpr point2d p0{1, 2};
    constexpr size2d sz{2u, 3u};
    constexpr auto p1 = p0 + narrow2d_cast<point2d<int>>(sz);

    constexpr rectangle rc{p0, sz};

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