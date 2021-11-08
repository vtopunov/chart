
#include <core/lerp.h>
#include <core/point2d.h>

void test_lerp() noexcept
{
    constexpr polynomial2 line_function{ 1, 2 };
    constexpr point2d p0{0, 1};
    constexpr point2d p1{1, 3};
    constexpr point2d p2{2, 5};
    constexpr num_range line01{ p0, p1 };
    constexpr num_range line12{ p1, p2 };
    constexpr num_range line02{ p0, p2 };

    constexpr auto line_function01 = lerp(line01);
    constexpr auto line_function12 = lerp(line12);
    constexpr auto line_function02 = lerp(line02);
    static_assert( line_function.coefficients == line_function01.coefficients );
    static_assert( line_function.coefficients == line_function12.coefficients );
    static_assert( line_function.coefficients == line_function02.coefficients );
    static_assert( line_function01( p2.x() ) == p2.y() );
    static_assert( line_function12( p0.x() ) == p0.y() );
    static_assert( line_function02( p1.x() ) == p1.y() );

    constexpr num_range argument_line01{ p0.x(), p1.x() };
    constexpr num_range value_line01{ p0.y(), p1.y() };
    constexpr auto line_function_av_01 = lerp(argument_line01, value_line01);

    static_assert(line_function.coefficients == line_function_av_01.coefficients);

    D_ASSERT( !errno );
}