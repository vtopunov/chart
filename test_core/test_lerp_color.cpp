#include <core/lerp.h>
#include <core/color.h>
#include <core/assert.h>

void test_lerp_color() noexcept
{
    constexpr auto animation_index = make_num_range( 0_z, 8_z );
    constexpr auto colors_from_cyan_to_red = make_num_range( colors::cyan, colors::red );
    constexpr auto animation_color_cyan_to_red = lerp( animation_index, colors_from_cyan_to_red );

    constexpr auto c0 = animation_color_cyan_to_red( 0 );
    constexpr auto c1 = animation_color_cyan_to_red( 1 );
    constexpr auto c2 = animation_color_cyan_to_red( 2 );
    constexpr auto c4 = animation_color_cyan_to_red( 4 );
    constexpr auto c6 = animation_color_cyan_to_red( 6 );
    constexpr auto c8 = animation_color_cyan_to_red( 8 );

    static_assert( c0 == colors::cyan );
    static_assert( c1 == color::from_rgb( 31, 223, 223 ) );
    static_assert( c2 == color::from_rgb( 63, 191, 191 ) );
    static_assert( c4 == color::from_rgb( 127, 127, 127 ) );
    static_assert( c6 == color::from_rgb( 191, 63, 63 ) );
    static_assert( c8 == colors::red );

    assert( !errno );
}