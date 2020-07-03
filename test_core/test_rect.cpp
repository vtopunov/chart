#include <core/rect.h>
#include <core/assert.h>

void test_rect() noexcept
{
    static_assert( std::is_trivial_v<rect<int>> && std::is_standard_layout_v<rect<int>> );

    const num_range r0{1, 3};
    const num_range r1{2, 5};
    const point min{1, 2};
    const point max{3, 5};
    const auto size = max - min;
    const rect rc{min, max};
    const auto rc_range = make_range_rect( r0, r1 );
    D_ASSERT( rc == rc_range );

    D_ASSERT( rc.diagonal.front() == min );
    D_ASSERT( rc.diagonal.back() == max );
    D_ASSERT( rc.includes( min ) );
    D_ASSERT( rc.includes( max ) );
    D_ASSERT( !rc.includes( max / 3 ) );
    D_ASSERT( rc.with_inclusion( max / 3 ).includes( max / 3 ) );
    D_ASSERT( rc.axis_range<axis_type::X>() == r0 );
    D_ASSERT( rc.axis_range<axis_type::Y>() == r1 );
    D_ASSERT( rc.x_axis_range() == r0 );
    D_ASSERT( rc.y_axis_range() == r1 );
    D_ASSERT( rc.size().measures == size );
    D_ASSERT( rc.width() == size.x() );
    D_ASSERT( rc.height() == size.y() );
    D_ASSERT( inverse_axis<axis_type::X>(rc).x_axis_range() == inverse(r0) );
    D_ASSERT( inverse_axis<axis_type::Y>(rc).y_axis_range() == inverse(r1) );

    D_ASSERT( !errno );
}