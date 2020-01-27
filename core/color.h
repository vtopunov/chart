#pragma once

#include <core/rational.h>

using color_byte_t = uint8_t;


template<class T>
constexpr color_byte_t real_to_color_byte(rational<T> real) noexcept
{
    D_ASSERT(real <= 1_ur);
    return narrow_cast<color_byte_t>(to_integer(real * max_v<color_byte_t>));
}

using argb_t = uint32_t;
using rgb_t = argb_t;

constexpr color_byte_t alpha_byte(argb_t argb) noexcept
{
    return narrow_cast<color_byte_t>(argb >> 24u);
}

constexpr color_byte_t red_byte(argb_t argb) noexcept
{
    return narrow_cast<color_byte_t>((argb >> 16u) & 0xffu);
}

constexpr color_byte_t green_byte(argb_t argb) noexcept
{
    return narrow_cast<color_byte_t>((argb >> 8u) & 0xffu);
}

constexpr color_byte_t blue_byte(argb_t argb) noexcept
{
    return narrow_cast<color_byte_t>(argb & 0xffu);
}

struct color
{
    color_byte_t blue;
    color_byte_t green;
    color_byte_t red;
    color_byte_t alpha;

    constexpr auto operator<=>(const color&) const noexcept = default;

    static constexpr color from_argb(color_byte_t a, color_byte_t r, color_byte_t g, color_byte_t b) noexcept
    {
        return { b, g, r, a };
    }

    static constexpr color from_argb(argb_t argb) noexcept
    {
        return from_argb(alpha_byte(argb), red_byte(argb), green_byte(argb), blue_byte(argb));
    }

    static constexpr color from_rgb(color_byte_t r, color_byte_t g, color_byte_t b) noexcept
    {
        return from_argb(max_v<color_byte_t>, r, g, b);
    }

    static constexpr color from_rgb(rgb_t rgb) noexcept
    {
        D_ASSERT(!alpha_byte(rgb));
        return from_rgb(red_byte(rgb), green_byte(rgb), blue_byte(rgb));
    }

    constexpr color with_opacity(color_byte_t opacity) const noexcept
    {
        color result{ *this };
        result.alpha = opacity;
        return result;
    }

    constexpr color with_transparency(color_byte_t transparency) const noexcept
    {
        return with_opacity(max_v<color_byte_t> -transparency);
    }

    template<class T>
    constexpr color with_opacity(rational<T> opacity) const noexcept
    {
        return with_opacity(real_to_color_byte(opacity));
    }

    template<class T>
    constexpr color with_transparency(rational<T> transparency) const noexcept
    {
        return with_transparency(real_to_color_byte(transparency));
    }

    constexpr argb_t to_argb() const noexcept
    {
        return static_cast<argb_t>(alpha) << 24
            | static_cast<argb_t>(red) << 16
            | static_cast<argb_t>(green) << 8
            | static_cast<argb_t>(blue);
    }
};

static_assert(sizeof(color) == 4);

constexpr color operator "" _rgb(unsigned long long rgb) noexcept
{
    return color::from_rgb(narrow_cast<rgb_t>(rgb));
}

constexpr color operator "" _argb(unsigned long long argb) noexcept
{
    return color::from_argb(narrow_cast<argb_t>(argb));
}

struct byte_overflowcolor
{
    using byte_overflow_type = rational<ptrdiff_t>;

    byte_overflow_type blue{ 0 };
    byte_overflow_type green{ 0 };
    byte_overflow_type red{ 0 };
    byte_overflow_type alpha{ 255 };

    constexpr byte_overflowcolor() noexcept = default;

    constexpr byte_overflowcolor
    (
        byte_overflow_type red_overflow, 
        byte_overflow_type green_overflow, 
        byte_overflow_type blue_overflow, 
        byte_overflow_type alpha_overflow
    ) noexcept
        : blue{ blue_overflow }
        , green{ green_overflow }
        , red{ red_overflow }
        , alpha{ alpha_overflow }
    {}

    constexpr byte_overflowcolor(color color) noexcept
        : blue{ color.blue }
        , green{ color.green }
        , red{ color.red }
        , alpha{ color.alpha }
    {}

    explicit constexpr operator color() const noexcept
    {
        return color::from_argb(
            narrow_cast<color_byte_t>(alpha.to_integer()),
            narrow_cast<color_byte_t>(red.to_integer()),
            narrow_cast<color_byte_t>(green.to_integer()),
            narrow_cast<color_byte_t>(blue.to_integer())
        );
    }
};

constexpr byte_overflowcolor operator + (byte_overflowcolor left, byte_overflowcolor right) noexcept
{
    return
    {
        left.red + right.red,
        left.green + right.green,
        left.blue + right.blue,
        left.alpha + right.alpha
    };
}

constexpr byte_overflowcolor operator - (byte_overflowcolor left, byte_overflowcolor right) noexcept
{
    return
    {
        left.red - right.red,
        left.green - right.green,
        left.blue - right.blue,
        left.alpha - right.alpha
    };
}

constexpr byte_overflowcolor operator * (byte_overflowcolor left, rational_t right) noexcept
{
    return
    {
        left.red * right,
        left.green * right,
        left.blue * right,
        left.alpha * right
    };
}

constexpr byte_overflowcolor operator * (rational_t left, byte_overflowcolor right) noexcept
{
    return right * left;
}

constexpr byte_overflowcolor operator / (byte_overflowcolor left, rational_t right) noexcept
{
    return
    {
        left.red / right,
        left.green / right,
        left.blue / right,
        left.alpha / right
    };
}

namespace colors
{
    constexpr color black = 0x000000_rgb;
    constexpr color gray = 0x808080_rgb;
    constexpr color white = 0xffffff_rgb;

    constexpr color red = 0xff0000_rgb;
    constexpr color green = 0x00ff00_rgb;
    constexpr color blue = 0x0000ff_rgb;

    constexpr color cyan = 0x00ffff_rgb;
    constexpr color magenta = 0xff00ff_rgb;
    constexpr color yellow = 0xffff00_rgb;
}