#include "texture.h"


template class unique_resource<gl::texture2d_resource, gl::texture_resource_deleter>;
template class unique_resource<gl::texture2d_resources, gl::texture_resource_deleter>;

namespace gl
{
    namespace
    {
        [[nodiscard]]
        constexpr bool is_sampler(glsl_typeid id) noexcept
        {
            return glsl_typeid::sampler2D == id || glsl_typeid::samplerCube == id;
        }

        [[nodiscard]]
        texture2d gen_texture() noexcept
        {
            texture_descriptor_t d{};
            glGenTextures(1, &d);
            return { resource_construct, d, size2d{ 0_px, 0_px } };
        }

        void set_image2d(pxsize2d sizes, texture_format format, const void* pixels) noexcept
        {
            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                to_underlying(format.format),
                narrow<GLsizei>(sizes.width()),
                narrow<GLsizei>(sizes.height()),
                0,
                to_underlying(format.format),
                to_underlying(format.type),
                pixels
            );
        }

        void set_default_parameteri2d() noexcept
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        template<texture_target Target>
        [[nodiscard]] bool bind(specialized_texture_resource<Target> texture) noexcept
        {
            bind_texture(texture);
            return is_correct();
        }
    }

    void texture_resource_deleter::operator()(texture_resource texture) const noexcept
    {
        glDeleteTextures(1, &texture.d);
    }


    texture2d create_texture2d() noexcept
    {
        if (auto tex = gen_texture(); tex && bind(view(tex))) [[likely]]
        {
            set_default_parameteri2d();
            if (is_correct()) [[likely]]
            {
                return tex;
            }
        }

        return {};
    }

    texture2d create_texture2d(pxsize2d sizes, texture_format format, const void* pixels) noexcept
    {
        if (auto tex = create_texture2d()) [[likely]]
        {
            set_image2d(sizes, format, pixels);
            if (is_correct()) [[likely]]
            {
                return gl::sizes(std::move(tex), sizes);
            }
        }

        return {};
    }

    void write(texture2d_resource tex, pxsize2d sizes, texture_format format, const void* pixels) noexcept
    {
        if (bind(tex)) [[likely]]
        {
            set_image2d(sizes, format, pixels);
        }
    }

    texture2d image(texture2d tex, pxsize2d sizes, texture_format format, const void* pixels) noexcept
    {
        write(tex, sizes, format, pixels);

        if (is_correct()) [[likely]]
        {
            tex = gl::sizes(std::move(tex), sizes);
        }

        return tex;
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
}