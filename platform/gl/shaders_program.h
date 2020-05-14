#pragma once

#include <core/span.h>
#include <core/handle.h>

#include <platform/gl/shaders_program_fwd.h>
#include <platform/gl/shader_fwd.h>

namespace gl
{
    namespace shaders_program
    {
        constexpr bool valid(program_view program) noexcept
        {
            return program != null_program;
        }

        inline void close(program_view program) noexcept
        {
            glDeleteProgram(program.handle);
        }

        inline void attach_shader(program_view program, shader_view shader) noexcept
        {
            glAttachShader(program.handle, shader.handle);
        }

        inline bool link(program_view program) noexcept
        {
            glLinkProgram(program.handle);
            if ( glGetError() == GL_NO_ERROR )
            {
                GLint status = 0;
                glGetProgramiv(program.handle, GL_LINK_STATUS, &status);
                return !!status;
            }
            return false;
        }

        inline void use(program_view program) noexcept
        {
            glUseProgram(program.handle);
        }

        span<const GLchar> info_log(program_view program, span<GLchar> buffer) noexcept
        {
            GLsizei log_length = 0;

            glGetProgramInfoLog
            (
                program.handle,
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

        using safe_program = unique_handle<program_view>;

        safe_program create_program() noexcept
        {
            return make_unique_handle<program_view>(glCreateProgram());
        }
    }

    using safe_shaders_program = shaders_program::safe_program;
}
