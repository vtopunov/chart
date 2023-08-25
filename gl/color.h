#pragma once

#include <core/color.h>

#include <gl/config.h>

namespace gl
{
    using rgba_colorf_t = rgba_color<GLfloat>;

    template<class T>
    [[nodiscard]] constexpr rgba_colorf_t to_colorf(const rgba_color<T>& source) noexcept
    {
        return color_cast<rgba_colorf_t>(source);
    }

    namespace colors
    {
        constexpr auto black_f = to_colorf(::colors::black);
        constexpr auto gray_f = to_colorf(::colors::gray);
        constexpr auto white_f = to_colorf(::colors::white);

        constexpr auto red_f = to_colorf(::colors::red);
        constexpr auto green_f = to_colorf(::colors::green);
        constexpr auto blue_f = to_colorf(::colors::blue);

        constexpr auto cyan_f = to_colorf(::colors::cyan);
        constexpr auto magenta_f = to_colorf(::colors::magenta);
        constexpr auto yellow_f = to_colorf(::colors::yellow);
    }

    namespace color_literals
    {
        [[nodiscard]]
        constexpr rgba_colorf_t operator "" _glrgb(unsigned long long rgb) noexcept
        {
            return gl::to_colorf(u32rgb_to_color(narrow_cast<u32argb_t>(rgb)));
        }

        [[nodiscard]]
        constexpr rgba_colorf_t operator "" _glargb(unsigned long long argb) noexcept
        {
            return gl::to_colorf(u32argb_to_color(narrow_cast<u32argb_t>(argb)));
        }
    }
}


using namespace gl::color_literals;