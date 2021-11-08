#include <core/num_range.h>
#include <core/assert.h>

void test_num_range() noexcept
{
    static_assert( std::is_trivial_v<num_range<int>> && std::is_standard_layout_v<num_range<int>> );

    constexpr num_range range{1, 3};

    static_assert( min(range) == 1 );
    static_assert( max(range) == 3 );

    {
        constexpr auto inv_r = inverse(range);
        static_assert(min(inv_r) == 1);
        static_assert(max(inv_r) == 3);
    }

    D_ASSERT( !errno );
}