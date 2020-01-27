#include <core/num_range.h>
#include <core/assert.h>

void test_num_range() noexcept
{
    constexpr auto range = make_num_range( 1, 3 );
    static_assert( range.min() == 1 );
    static_assert( range.max() == 3 );
    static_assert( range.front() == 1 );
    static_assert( range.back() == 3 );
    static_assert( !range.includes( 0 ) );
    static_assert( range.includes( 1 ) );
    static_assert( range.includes( 2 ) );
    static_assert( range.includes( 3 ) );
    static_assert( !range.includes( 4 ) );

    {
        constexpr auto inverse_range = inverse(range);
        static_assert( inverse_range.min() == 1 );
        static_assert( inverse_range.max() == 3 );
        static_assert( inverse_range.front() == 3 );
        static_assert( inverse_range.back() == 1 );
    }

    {
        constexpr auto range_with_4 = range.with_inclusion( 4 );
        static_assert( range_with_4.includes( 4 ) );
        static_assert( range_with_4.includes( 1 ) );
        static_assert( !range_with_4.includes( 0 ) );
    }

    {
        constexpr auto range_moving_1 = range.with_moving( 1 );
        static_assert( range_moving_1.includes( 4 ) );
        static_assert( !range_moving_1.includes( 1 ) );
    }

    D_ASSERT( !errno );
}