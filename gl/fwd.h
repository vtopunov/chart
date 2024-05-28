#pragma once

#include <core/fwd.h>

#include <gl/config.h>


namespace gl
{
    static_assert(std::is_same_v<luminancef_t, GLfloat>);

    using vec2i = vec2<GLint>;
    using vec2f = vec2<GLfloat>;
    using vec2b = vec2<GLboolean>;

    using const_span2i = span<const GLint, 2_uz>;
    using const_span2f = span<const GLfloat, 2_uz>;
    using const_span2b = span<const GLboolean, 2_uz>;

    using const_span3i = span<const GLint, 3_uz>;
    using const_span3f = span<const GLfloat, 3_uz>;
    using const_span3b = span<const GLboolean, 3_uz>;

    using const_span4i = span<const GLint, 4_uz>;
    using const_span4f = span<const GLfloat, 4_uz>;
    using const_span4b = span<const GLboolean, 4_uz>;
}
