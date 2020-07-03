#include <core/point.h>
#include <core/lerp.h>

#include <core/assert.h>

void test_lerp() noexcept
{
    constexpr polynomial line_function{ 1, 2 };
    constexpr point p0{0, 1};
    constexpr point p1{1, 3};
    constexpr point p2{2, 5};

    constexpr auto line_function_01 = lerp( p0, p1 );
    constexpr auto line_function_12 = lerp( p1, p2 );
    constexpr auto line_function_02 = lerp( p0, p2 );
    static_assert( line_function.coefficients == line_function_01.coefficients );
    static_assert( line_function.coefficients == line_function_12.coefficients );
    static_assert( line_function.coefficients == line_function_02.coefficients );
    static_assert( line_function_01( p2.x() ) == p2.y() );
    static_assert( line_function_12( p0.x() ) == p0.y() );
    static_assert( line_function_02( p1.x() ) == p1.y() );

    D_ASSERT( !errno );
}