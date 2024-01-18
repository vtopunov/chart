#include <core/lerp.h>

#include <core/fwd.h>

#include <cerrno>


void test_lerp() noexcept
{
    constexpr polynomial2 line_function{ 2, 1 };

    constexpr auto line_function_01 = lerp(0, 1, 1, 3);
    constexpr auto line_function_12 = lerp(1, 2, 3, 5);
    constexpr auto line_function_02 = lerp(0, 2, 1, 5);

    static_assert(line_function == line_function_01);
    static_assert(line_function == line_function_12);
    static_assert(line_function == line_function_02);

    D_ASSERT( !errno );
}