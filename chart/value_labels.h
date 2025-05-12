#pragma once

#include <core/small_vector.h>

#include <utility/font_cache.h>

#include <shader/library.h>

#include <chart/space_manipulation.h>
#include <chart/periodic_position.h>


namespace chart
{
    struct value_labels
    {
        static constexpr pxmargins default_margins
        {
            .left{ 60_npx },
            .top{ 10_npx },
            .right{ 10_npx },
            .bottom{ 30_npx }
        };

        pxmargins margins{ default_margins };
        font_cache::face font{};
        gl::texture2d x_axis_texture_cache{};
        gl::texture2d y_axis_texture_cache{};
        real_t x_axis_offset_cache{ 0.0 };
        real_t y_axis_offset_cache{ 0.0 };

        constexpr void operator () (pxrectangle& geometry) const noexcept
        {
            geometry.position.ref_x() += margins.left;
            geometry.position.ref_y() += margins.top;
            geometry.sizes.ref_width() -= (margins.left + margins.right);
            geometry.sizes.ref_height() -= (margins.top + margins.bottom);
        }

        void operator () (const periodic_value_position& position, buffer_view buffer) noexcept
        {
            
        }

        void operator () (const shader_embed::luminance_texture& shdr, const pxrectangle& geometry) noexcept
        {

        }
    };
}



