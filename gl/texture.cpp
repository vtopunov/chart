#include "texture.h"

namespace gl
{
    namespace
    {
        [[nodiscard]]
        constexpr bool is_sampler(glsl_typeid id) noexcept
        {
            return glsl_typeid::sampler2D == id || glsl_typeid::samplerCube == id;
        }

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
                narrow_cast<GLsizei>(sizes.width()),
                narrow_cast<GLsizei>(sizes.height()),
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

        template<class T>
        void bind(const unique_resource<T, texture_resource_deleter>& r) noexcept
        {
            bind_texture(T::target, view(r));
        }
    }

    void texture_resource_deleter::operator()(texture_resource texture) const noexcept
    {
        glDeleteTextures(1, &texture.d);
    }


    texture2d create_texture2d() noexcept
    {
        auto tex = gen_texture();
        if (D_UNLIKELY(has_error())) D_ATTRIB_UNLIKELY
        {
            return {};
        }

        bind(tex);
        if (D_UNLIKELY(has_error())) D_ATTRIB_UNLIKELY
        {
            return {};
        }

        set_default_parameteri2d();
        if (D_UNLIKELY(has_error())) D_ATTRIB_UNLIKELY
        {
            return {};
        }

        return tex;
    }

    texture2d create_texture2d(pxsize2d sizes, texture_format format, const void* pixels) noexcept
    {
        auto tex = create_texture2d();

        if (tex)
        {
            set_image2d(sizes, format, pixels);

            if (D_LIKELY(is_correct())) D_ATTRIB_LIKELY
            {
                tex =
                {
                    resource_construct,
                    tex.release(),
                    sizes
                };
            }
            else
            {
                tex.reset();
            }
        }

        return tex;
    }

    texture2d write(texture2d tex, pxsize2d sizes, texture_format format, const void* pixels) noexcept
    {
        bind(tex);

        if (D_LIKELY(is_correct())) D_ATTRIB_LIKELY
        {
            set_image2d(sizes, format, pixels);

            if (D_LIKELY(is_correct())) D_ATTRIB_LIKELY
            {
                tex =
                {
                    resource_construct,
                    tex.release(),
                    sizes
                };
            }
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
