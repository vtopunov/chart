#pragma once

#include <gl/color.h>
#include <gl/texture.h>

#include <widget/rectangle.h>


namespace widget
{
    namespace shader
    {
        namespace colored_rectangle
        {
            bool initialize(px::size2d viewport_sizes) noexcept;

            void draw(rectangle rc, gl::rgba_colorf_t colorf) noexcept;
        }

        namespace gray_texture_mix_color
        {
            bool initialize(px::size2d viewport_sizes) noexcept;

            void draw(px::point2d position, gl::texture2d_resources texture, gl::rgba_colorf_t colorf) noexcept;
        }
    }
}
