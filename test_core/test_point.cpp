#include <core/point.h>
#include <core/assert.h>

void test_point() noexcept
{
    constexpr auto p0 = make_point( 3, 5 );
    constexpr auto p0_minus = make_point( -3, -5 );
    static_assert( p0.x() == 3 );
    static_assert( p0.y() == 5 );
    static_assert( p0.get<axis_type::X>() == p0.x() );
    static_assert( p0.get<axis_type::Y>() == p0.y() );

    constexpr auto p1 = make_point( 1, 2 );
    constexpr auto p2 = make_point( 2, 3 );
    static_assert( p1 + p2 == p0 );
    static_assert( p2 + p1 == p0 );
    static_assert( -p0 == p0_minus );
    static_assert( -( -p0 ) == p0 );
    static_assert( p0 - p1 == p2 );
    static_assert( p0 - p2 == p1 );
    static_assert( p1 - p0 == -p2 );
    static_assert( p1 * 2 == p1 + p1 );
    static_assert( 2 * p1 == p1 + p1 );
    static_assert( ( p0 + p1 + p2 ) / 2 == p0 );

    assert( !errno );
}