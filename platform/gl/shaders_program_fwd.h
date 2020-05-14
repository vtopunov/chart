#pragma once

#include <compare>

#include <platform/gl/config.h>

namespace gl
{
    namespace shaders_program
    {
        struct program_view
        {
            GLuint handle;

            constexpr auto operator<=>(const program_view&) const noexcept = default;
        };

        constexpr program_view null_program{ 0u };
    }
}