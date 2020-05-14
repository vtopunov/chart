#pragma once

#include <core/assert.h>
#include <core/underlying_cast.h>
#include <core/span.h>
#include <core/handle.h>

#include <platform/gl/shader_fwd.h>

namespace gl
{
    constexpr bool valid(shader_view shader) noexcept
    {
        return shader != null_shader;
    }

    inline void close(shader_view shader) noexcept
    {
        if ( valid(shader) )
        {
            glDeleteShader(shader.handle);
        }
    }

    inline void set_source(shader_view shader, span<const GLchar> source) noexcept
    {
        const auto source_data = source.data();
        const auto source_length = narrow_cast<GLint>( source.size() );
        glShaderSource(shader.handle, 1, &source_data, &source_length);
    }

    inline bool compile(shader_view shader) noexcept
    {
        glCompileShader(shader.handle);

        GLint status = 0;
        glGetShaderiv(shader.handle, GL_COMPILE_STATUS, &status);
        return !!status;
    }

    inline bool compile(shader_view shader, span<const GLchar> source) noexcept
    {
        set_source(shader, source);
        return compile(shader);
    }

    inline span<const GLchar> info_log(shader_view shader, span<GLchar> buffer) noexcept
    {
        GLsizei log_length = 0;
        glGetShaderInfoLog
        (
            shader.handle, 
            narrow_cast<GLsizei>( buffer.size() ), 
            &log_length, 
            buffer.data()
        );

        return 
        { 
            buffer.data(), 
            narrow_cast<size_t>( log_length ) 
        };
    }

    enum class shader_type : GLenum
    {
        fragment = GL_FRAGMENT_SHADER,
        vertex = GL_VERTEX_SHADER
    };

    using safe_shader = unique_handle<shader_view>;

    inline safe_shader create_shader(shader_type type) noexcept
    {
        return make_unique_handle<shader_view>(glCreateShader(underlying_cast<GLenum>(type)));
    }
}
