#pragma once

#include "rational.h"

constexpr byte_t real_to_color_byte( urational_t real ) noexcept
{
    assert( real <= 1_ur );
    return narrow_cast<color_byte_t>( to_integer( real * max_v<color_byte_t> ) );
}

struct color
{
    byte_t blue{ 0 };
    byte_t green{ 0 };
    byte_t red{ 0 };
    byte_t alpha{ 255 };

    constexpr color() noexcept = default;

    constexpr color( byte_t red_byte, byte_t green_byte, byte_t blue_byte, byte_t alpha_byte ) noexcept
        : blue{ blue_byte }
        , green{ green_byte }
        , red{ red_byte }
        , alpha{ alpha_byte }
    {}

    constexpr color( byte_t red_byte, byte_t green_byte, byte_t blue_byte ) noexcept
        : blue{ blue_byte }
        , green{ green_byte }
        , red{ red_byte }
    {}

    constexpr color with_opacity(byte_t opacity) const noexcept
    {
        color result{ *this };
        result.alpha = opacity;
        return result;
    }

    constexpr color with_transparency(byte_t transparency) const noexcept
    {
        return with_opacity(max_v<byte_t> - transparency);
    }

    constexpr color with_opacity( urational_t opacity ) const noexcept
    {
        return with_opacity( real_to_color_byte( opacity ) );
    }

    constexpr color with_transparency( urational_t transparency ) const noexcept
    {
        return with_transparency( real_to_color_byte( transparency ) );
    }

    uint32_t to_uint() const noexcept
    {
        return bit_cast<uint32_t>( *this );
    }
};

static_assert( sizeof( color ) == 4 );

constexpr bool operator == ( color left, color rigth ) noexcept
{
    // left.to_uint() == rigth.to_uint(); c++20
    return left.red == rigth.red &&
        left.green == rigth.green &&
        left.blue == rigth.blue &&
        left.alpha == rigth.alpha;
}

constexpr bool operator != ( color left, color rigth ) noexcept
{
    return !( left == rigth );
}


struct byte_overflow_color
{
    rational_t blue{ 0 };
    rational_t green{ 0 };
    rational_t red{ 0 };
    rational_t alpha{ 255 };

    constexpr byte_overflow_color() noexcept = default;

    constexpr byte_overflow_color( rational_t red_overflow, rational_t green_overflow, rational_t blue_overflow, rational_t alpha_overflow ) noexcept
        : blue{ blue_overflow }
        , green{ green_overflow }
        , red{ red_overflow }
        , alpha{ alpha_overflow }
    {}

    constexpr byte_overflow_color( color color ) noexcept
        : blue{ color.blue }
        , green{ color.green }
        , red{ color.red }
        , alpha{ color.alpha }
    {}

    explicit constexpr operator color() const noexcept
    {
        return
        {
            narrow_cast<byte_t>( red.to_integer() ),
            narrow_cast<byte_t>( blue.to_integer() ),
            narrow_cast<byte_t>( green.to_integer() ),
            narrow_cast<byte_t>( alpha.to_integer() )
        };
    }
};


constexpr byte_overflow_color operator + ( byte_overflow_color left, byte_overflow_color right ) noexcept
{
    return
    {
        left.red + right.red,
        left.blue + right.blue,
        left.green + right.green,
        left.alpha + right.alpha
    };
}

constexpr byte_overflow_color operator - ( byte_overflow_color left, byte_overflow_color right ) noexcept
{
    return
    {
        left.red - right.red,
        left.blue - right.blue,
        left.green - right.green,
        left.alpha - right.alpha
    };
}

constexpr byte_overflow_color operator * ( byte_overflow_color left, rational_t right ) noexcept
{
    return
    {
        left.red * right,
        left.blue * right,
        left.green * right,
        left.alpha * right
    };
}

constexpr byte_overflow_color operator * ( rational_t left, byte_overflow_color right ) noexcept
{
    return right * left;
}

constexpr byte_overflow_color operator / ( byte_overflow_color left, rational_t right ) noexcept
{
    return
    {
        left.red / right,
        left.blue / right,
        left.green / right,
        left.alpha / right
    };
}

namespace colors
{
    constexpr color black{ 0, 0, 0 };
    constexpr color gray{ 128, 128, 128 };
    constexpr color white{ 255, 255, 255 };

    constexpr color red{ 255, 0, 0 };
    constexpr color green{ 0, 255, 0 };
    constexpr color blue{ 0, 0, 255 };

    constexpr color cyan{ 0, 255, 255 };
    constexpr color magenta{ 255, 0, 255 };
    constexpr color yellow{ 255, 255, 0 };
}