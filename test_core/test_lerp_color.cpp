#include <core/lerp.h>
#include <core/color.h>
#include <core/assert.h>

void test_lerp_color() noexcept
{
    constexpr num_range animation_index{0, 8};
    constexpr num_range colors_from_cyan_to_red{colors::cyan, colors::red};
    constexpr auto animation_color_cyan_to_red = lerp(animation_index, colors_from_cyan_to_red);

    constexpr auto c0 = animation_color_cyan_to_red(0);
    constexpr auto c1 = animation_color_cyan_to_red(1);
    constexpr auto c2 = animation_color_cyan_to_red(2);
    constexpr auto c4 = animation_color_cyan_to_red(4);
    constexpr auto c6 = animation_color_cyan_to_red(6);
    constexpr auto c8 = animation_color_cyan_to_red(8);

    static_assert( color_cast<argb_color32_t>(c0 ) == colors::cyan );
    static_assert( color_cast<argb_color32_t>( c1 ) == argb_color32_t::instance(31, 223, 223) );
    static_assert( color_cast<argb_color32_t>( c2 )  == argb_color32_t::instance(63, 191, 191) );
    static_assert( color_cast<argb_color32_t>( c4 ) == argb_color32_t::instance(127, 127, 127) );
    static_assert( color_cast<argb_color32_t>( c6 ) == argb_color32_t::instance(191, 63, 63) );
    static_assert( color_cast<argb_color32_t>( c8 ) == colors::red );

    D_ASSERT(!errno);
}