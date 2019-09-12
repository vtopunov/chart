#include <core/rect.h>
#include <core/assert.h>

void test_rect() noexcept
{
    constexpr numerical_range r0{ 1, 3 };
    constexpr numerical_range r1{ 2, 5 };
    constexpr point min{ 1, 2 };
    constexpr point max{ 3, 5 };
    constexpr point size{ max - min };
    constexpr rect rc{ min, max };
    constexpr rect rc_range{ r0, r1 };
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
    static_assert( rc.with_inverse_axis<axis_type::X>().x_axis_range() == r0.with_inverse() );
    static_assert( rc.with_inverse_axis<axis_type::Y>().y_axis_range() == r1.with_inverse() );

    assert( !errno );
}