#include <core/lerp.h>
#include <core/color.h>


void test_lerp_color() noexcept
{
    constexpr auto animation_color_cyan_to_red = lerp
    (
        0, 8, 
        colors::cyan, colors::red
    );

    constexpr auto c0 = animation_color_cyan_to_red(0);
    constexpr auto c1 = animation_color_cyan_to_red(1);
    constexpr auto c2 = animation_color_cyan_to_red(2);
    constexpr auto c4 = animation_color_cyan_to_red(4);
    constexpr auto c6 = animation_color_cyan_to_red(6);
    constexpr auto c8 = animation_color_cyan_to_red(8);

    static_assert( color_cast<rgba_color32_t>(c0 ) == colors::cyan );
    static_assert( color_cast<rgba_color32_t>( c1 ) == rgba_color32_t{31, 223, 223, tint_max_v<>} );
    static_assert( color_cast<rgba_color32_t>( c2 )  == rgba_color32_t{63, 191, 191, tint_max_v<>} );
    static_assert( color_cast<rgba_color32_t>( c4 ) == rgba_color32_t{127, 127, 127, tint_max_v<>} );
    static_assert( color_cast<rgba_color32_t>( c6 ) == rgba_color32_t{191, 63, 63, tint_max_v<>} );
    static_assert( color_cast<rgba_color32_t>( c8 ) == colors::red );

    D_ASSERT(!errno);
}