#include <core/rect.h>
#include <core/assert.h>

void test_rect() noexcept
{
    constexpr auto r0 = make_num_range( 1, 3 );
    constexpr auto r1 = make_num_range( 2, 5 );
    constexpr auto min = make_point( 1, 2 );
    constexpr auto max = make_point( 3, 5 );
    constexpr auto size = max - min;
    constexpr auto rc = make_rect( min, max );
    constexpr auto rc_range = make_rect( r0, r1 );
    static_assert( rc == rc_range );

    static_assert( rc.diagonal.front() == min );
    static_assert( rc.diagonal.back() == max );
    static_assert( rc.includes( min ) );
    static_assert( rc.includes( max ) );
    static_assert( !rc.includes( max / 3 ) );
    static_assert( rc.with_inclusion( max / 3 ).includes( max / 3 ) );
    static_assert( rc.axis_range<axis_type::X>() == r0 );
    static_assert( rc.axis_range<axis_type::Y>() == r1 );
    static_assert( rc.x_axis_range() == r0 );
    static_assert( rc.y_axis_range() == r1 );
    static_assert( rc.size() == size );
    static_assert( rc.width() == size.x() );
    static_assert( rc.height() == size.y() );
    static_assert( inverse_axis<axis_type::X>(rc).x_axis_range() == inverse(r0) );
    static_assert( inverse_axis<axis_type::Y>(rc).y_axis_range() == inverse(r1) );

    assert( !errno );
}