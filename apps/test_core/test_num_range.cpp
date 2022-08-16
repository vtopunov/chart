#include <core/num_range.h>
#include <core/assert.h>

void test_num_range() noexcept
{
    static_assert( std::is_trivial_v<num_range<int>> && std::is_standard_layout_v<num_range<int>> );

    constexpr num_range range{1, 3};
    static_assert(2 == range.length());

    D_ASSERT( !errno );
}