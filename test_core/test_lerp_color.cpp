#include <core/lerp.h>
#include <core/color.h>
#include <core/assert.h>

void test_lerp_color() noexcept
{
    constexpr numerical_range animation_index{ 0LL, 8LL };
    constexpr numerical_range colors_from_cyan_to_red{ colors::cyan, colors::red };
    constexpr auto animation_color_cyan_to_red = lerp( animation_index, colors_from_cyan_to_red );

    constexpr auto c0 = animation_color_cyan_to_red( 0 );
    constexpr auto c1 = animation_color_cyan_to_red( 1 );
    constexpr auto c2 = animation_color_cyan_to_red( 2 );
    constexpr auto c4 = animation_color_cyan_to_red( 4 );
    constexpr auto c6 = animation_color_cyan_to_red( 6 );
    constexpr auto c8 = animation_color_cyan_to_red( 8 );

    static_assert( c0 == colors::cyan );
    static_assert( c1 == color( 31, 223, 223 ) );
    static_assert( c2 == color( 63, 191, 191 ) );
    static_assert( c4 == color( 127, 127, 127 ) );
    static_assert( c6 == color( 191, 63, 63 ) );
    static_assert( c8 == colors::red );

    assert( !errno );
}