#include <core/color.h>
#include <core/assert.h>

void test_color() noexcept
{
    constexpr auto c0 = color::from_argb(0x04, 0x01, 0x02, 0x03);
    assert(c0.red == 0x01);
    assert(c0.green == 0x02);
    assert(c0.blue == 0x03);
    assert(c0.alpha == 0x04);
    static_assert(c0.to_argb() == 0x04010203ul);

    constexpr auto c1 = color::from_rgb(0x01, 0x02, 0x03);
    assert(c1.red == 0x01);
    assert(c1.green == 0x02);
    assert(c1.blue == 0x03);
    assert(c1.alpha == 0xff);
    assert(c1.to_argb() == 0xff010203ul);

    constexpr color c2 = 0x04010203_argb;
    static_assert(c2 == c0);

    constexpr color c3 = 0x010203_rgb;
    assert(c3 == c1);

    assert(!errno);
}