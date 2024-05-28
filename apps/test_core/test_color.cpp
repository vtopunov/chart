#include <core/color.h>


template<class T, class A, class R, class G, class B>
void test_argb(const basic_rgba_color<T>& c, A a, R r, G g, B b) noexcept
{
    D_ASSERT(c.a == a);
    D_ASSERT(c.r == r);
    D_ASSERT(c.g == g);
    D_ASSERT(c.b == b);
}


template<class L, class R>
constexpr bool eq(const basic_rgba_color<L>& l, const basic_rgba_color<R>& r) noexcept
{
    using common_t = std::common_type_t<L, R>;
    using color_common_t = basic_rgba_color<common_t>;
    return static_cast<color_common_t>(l) == static_cast<color_common_t>(r);
}

void test_color() noexcept
{
    using namespace color_literals;
    using namespace rational_literals;

    static_assert(std::is_trivial_v<rgba_color> && std::is_standard_layout_v<rgba_color>);

    static_assert(is_rgba_color_v< rgba_color >);
    static_assert(is_rgba_color_v< const rgba_color >);
    static_assert(!is_rgba_color_v< luminance_t >);
    static_assert(!is_rgba_color_v< argb_t >);
    static_assert(!is_rgba_color_v< float >);
    static_assert(!is_rgba_color_v< span<luminance_t> >);
    static_assert(!is_rgba_color_v< span<luminance_t, 4u> >);
    static_assert(!is_rgba_color_v< span<const luminance_t> >);
    static_assert(!is_rgba_color_v< span<const luminance_t, 4u> >);
    static_assert(std::is_same_v<decl_value_type_t<rgba_color>, luminance_t>);
    static_assert(std::is_same_v<decl_view_type_t<rgba_color>, span<const luminance_t, 4u> >);
    static_assert(rgba_color_extent == 4u);
    static_assert(extent_v<rgba_color_view> == 4u);
    static_assert(extent_v<rgba_color> == 4u);
    static_assert(rgba_color::extent == 4u);
    static_assert(sizeof(rgba_color) == 4u);
    static_assert(colors::gray.r == (colors::white.r / 2));

    {
        constexpr rgba_color c{ 0x01, 0x02, 0x03, 0x04 };
        test_argb(c, 0x04, 0x01, 0x02, 0x03);
        static_assert(color_cast<argb_t>(c) == 0x04010203ul);
        static_assert(color_cast<rgba_color>(0x04010203ul) == c);

        const span c_view{ c };
        D_ASSERT(color_cast<rgba_color>(c_view) == c);
        D_ASSERT(color_cast<argb_t>(c_view) == 0x04010203ul);
    }

    {
        constexpr luminance_t color_array[] = { 0x01, 0x02, 0x03, 0x04 };
        constexpr auto c = color_cast<rgba_color>(color_array);
        test_argb(c, 0x04, 0x01, 0x02, 0x03);
        static_assert(color_cast<argb_t>(color_array) == 0x04010203ul);

        const span c_view{ color_array };
        D_ASSERT(color_cast<rgba_color>(c_view) == c);
        D_ASSERT(color_cast<argb_t>(c_view) == 0x04010203ul);
    }

    {
        constexpr std::array<luminance_t, rgba_color_extent> color_array{ 0x01, 0x02, 0x03, 0x04 };
        constexpr auto c = color_cast<rgba_color>(color_array);
        test_argb(c, 0x04, 0x01, 0x02, 0x03);
        static_assert(color_cast<argb_t>(color_array) == 0x04010203ul);

        const span c_view{ color_array };
        D_ASSERT(color_cast<rgba_color>(c_view) == c);
        D_ASSERT(color_cast<argb_t>(c_view) == 0x04010203ul);
    }

    {
        constexpr rgba_color c{ 0x01, 0x02, 0x03 };
        test_argb(c, 0x00, 0x01, 0x02, 0x03);
        static_assert(color_cast<argb_t>(c) == 0x00010203ul);
        static_assert(color_cast<rgba_color>(0x00010203ul) == c);
        constexpr auto c2 = c.with_alpha(0xff);
        test_argb(c2, 0xff, 0x01, 0x02, 0x03);
        static_assert(color_cast<argb_t>(c2) == 0xff010203ul);
        static_assert(color_cast<rgba_color>(0xff010203ul) == c2);
    }

    test_argb(colors::red, 0xff, 0xff, 0, 0);
    test_argb(colors::green, 0xff, 0, 0xff, 0);
    test_argb(colors::blue, 0xff, 0, 0, 0xff);

    constexpr auto redf = color_cast<rgbaf_color>(colors::red);
    test_argb(redf, 1.0f, 1.0f, 0.0f, 0.0f);

    constexpr auto greenf = color_cast<rgbaf_color>(colors::green);
    test_argb(greenf, 1.0f, 0.0f, 1.0f, 0.0f);

    constexpr auto bluef = color_cast<rgbaf_color>(colors::blue);
    test_argb(bluef, 1.0f, 0.0f, 0.0f, 1.0f);

    static_assert(color_cast<rgbaf_color>(0xffff0000ul) == redf);
    static_assert(color_cast<rgbaf_color>(0xff00ff00ul) == greenf);
    static_assert(color_cast<rgbaf_color>(0xff0000fful) == bluef);

    {
        constexpr auto c = 0x04010203_argb;
        test_argb(c, 0x04, 0x01, 0x02, 0x03);
    }

    {
        constexpr auto c = 0x010203_rgb;
        test_argb(c, 0xff, 0x01, 0x02, 0x03);
    }

    {
        test_argb((2 * colors::red) / 3, 255_r, 170_r, 0_r, 0_r);
        test_argb((2 * colors::green) / 3, 255_r, 0_r, 170_r, 0_r);
        test_argb((2 * colors::blue) / 3, 255_r, 0_r, 0_r, 170_r);

        test_argb(colors::cyan, 0xff, 0, 0xff, 0xff);
        test_argb(colors::magenta, 0xff, 0xff, 0, 0xff);
        test_argb(colors::yellow, 0xff, 0xff, 0xff, 0);

        static_assert(eq(colors::cyan, colors::green + colors::blue));
        static_assert(eq(colors::magenta, colors::red + colors::blue));
        static_assert(eq(colors::yellow, colors::red + colors::green));
        static_assert(eq(colors::cyan - colors::green, colors::blue));
        static_assert(eq(colors::cyan - colors::blue, colors::green));
        static_assert(eq(colors::magenta - colors::red, colors::blue));
        static_assert(eq(colors::magenta - colors::blue, colors::red));
        static_assert(eq(colors::yellow - colors::red, colors::green));
        static_assert(eq(colors::yellow - colors::green, colors::red));
    }
}