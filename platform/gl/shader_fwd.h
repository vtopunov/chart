#pragma once

#include <compare>

#include <platform/gl/config.h>

namespace gl
{
    struct shader_view
    {
        GLuint handle;

        constexpr auto operator<=>(const shader_view&) const noexcept = default;
    };

    constexpr shader_view null_shader{ 0u };
}