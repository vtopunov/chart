#include "texture.h"

namespace gl
{
    namespace
    {
        template<texture_target target>
        void set_texture(specialized_texture_resource<target> texture, px::size2d sizes, texture_format format, const void* pixels) noexcept
        {
            texture.bind();

            constexpr auto gl_target = to_underlying(target);
            glTexParameteri(gl_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(gl_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D
            (
                gl_target,
                0,
                to_underlying(format.format),
                narrow_cast<GLsizei>(sizes.width()),
                narrow_cast<GLsizei>(sizes.height()),
                0,
                to_underlying(format.format),
                to_underlying(format.type),
                pixels
            );
        }

        texture_descriptor_t gen_texture() noexcept
        {
            texture_descriptor_t d{};
            glGenTextures(1, &d);
            return d;
        }
    }

    void texture_resource_deleter::operator()(texture_resource texture) const noexcept
    {
        glDeleteTextures(1, &texture.d);
    }

    texture2d create_texture2d(px::size2d sizes, texture_format format, const void* pixels) noexcept
    {
        texture2d texture
        {
            resource_construct,
            gen_texture(),
            sizes
        };

        D_ASSERT(texture);

        set_texture(view(texture), sizes, format, pixels);

        return texture;
    }


    std::underlying_type_t<uniform_location> get_sampler_number(shaders_program_resource program, uniform_location location) noexcept
    {
        std::underlying_type_t<uniform_location> number{ 0u };

        for (auto locaion = to_underlying(location); locaion; )
        {
            --locaion;

            GLenum type_id{ 0 };
            GLint size{ 0 };
            glGetActiveUniform(to_underlying(program), locaion, 0, nullptr, &size, &type_id, nullptr);

            if (is_sampler(safe_numeric_cast<glsl_typeid>(type_id)))
            {
                ++number;
            }
        }

        return number;
    }
}
