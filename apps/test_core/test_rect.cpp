#include <core/rect.h>
#include <core/assert.h>

void test_rect() noexcept
{
    static_assert( std::is_trivial_v<rect<int>> && std::is_standard_layout_v<rect<int>> );

    constexpr vec2 p0{1, 2};
    constexpr vec2 p1{3, 5};

    constexpr rect rc{p0, p1};

    constexpr num_range line01{ p0, p1 };
    constexpr auto center01 = ( p0 + p1 ) / 2;
    constexpr auto sizes01 = p1 - p0;

    static_assert( rc.diagonal == line01 );
    static_assert( rc.p00() == p0 );
    static_assert( rc.p01() == vec2{ p0.x(), p1.y() } );
    static_assert( rc.p10() == vec2{ p1.x(), p0.y() } );
    static_assert( rc.p11() == p1 );
    static_assert( rc.center() == center01 );
    static_assert( rc.x0() == p0.x() );
    static_assert( rc.y0() == p0.y() );
    static_assert( rc.x1() == p1.x() );
    static_assert( rc.y1() == p1.y() );
    static_assert( rc.sizes() == sizes01 );

    D_ASSERT( !errno );
}