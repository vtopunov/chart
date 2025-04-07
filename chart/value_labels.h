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
        font_cache::face font{};
       
        void operator () (const periodic_value_position&, buffer_view buffer) noexcept
        {
            D_ASSERT(buffer);
        }

        void operator () (const periodic_value_position&, const shader_embed::luminance_texture& shdr) noexcept
        {
            D_ASSERT(gl::program_resource::null != shdr.program());
        }
    };
}



