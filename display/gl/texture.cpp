#include "texture.h"

namespace display
{
    namespace gl
    {
        namespace
        {
            void texture_image2D(texture_target target, size_t width, size_t height, texture_format format, const void* pixels) noexcept
            {
                glTexImage2D
                (
                    to_underlying(target),
                    0,
                    to_underlying(format.format),
                    narrow_cast<GLsizei>(width),
                    narrow_cast<GLsizei>(height),
                    0,
                    to_underlying(format.format),
                    to_underlying(format.type),
                    pixels
                );
            }

            void texture_parameter(texture_target target, GLenum name, GLint value) noexcept
            {
                glTexParameteri(to_underlying(target), name, value);
            }

            texture_descriptor_t gen_texture() noexcept
            {
                texture_descriptor_t d{};
                glGenTextures(1, &d);
                return d;
            }
            
        }

        void texture_resource_deleter::operator()(texture_resource texture, resource_destroy_t) const noexcept
        {
            glDeleteTextures(1, &texture.d);
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

                if (is_sampler(underlying_cast<glsl_typeid>(type_id)))
                {
                    ++number;
                }
            }

            return number;
        }

        texture2d_t create_texture2d(size_t width, size_t height, texture_format format, const void* pixels) noexcept
        {
            using texture_t = texture2d_t;
            using texture_resource_t = typename texture_t::resource_type;
            constexpr auto target = texture_resource_t::target;

            texture_t texture
            {
                resource_construct,
                gen_texture()
            };

            D_ASSERT(texture);

            texture->bind();
            
            texture_parameter(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            texture_parameter(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            texture_image2D(target, width, height, format, pixels);

            return texture;
        }
    }
}