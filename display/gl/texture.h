#pragma once

#include <display/gl/shader.h>

namespace display
{
    namespace gl
    {
        using texture_descriptor_t = GLuint;

        struct texture_resource
        {
            texture_descriptor_t d;

            [[nodiscard]]
            constexpr explicit operator bool() const noexcept
            {
                return !!d;
            }
        };

        struct texture_resource_deleter
        {
            void operator () (texture_resource texture, resource_destroy_t) const noexcept;
        };

        enum class texture_target : GLenum
        {
            texture_2d = GL_TEXTURE_2D
        };

        template<texture_target binded_target>
        struct binded_texture_resource : texture_resource
        {
            static constexpr auto target = binded_target;
        };

        using texture_resource2d_t = binded_texture_resource<texture_target::texture_2d>;

        using texture2d_t = unique_resource<texture_resource2d_t, texture_resource_deleter>;

        inline void bind(texture_resource2d_t texture) noexcept
        {
            glBindTexture(to_underlying(texture.target), texture.d);
        }

        enum class pixel_format : GLenum
        {
            RGBA = GL_RGBA,
            RGB = GL_RGB,
            LUMINANCE = GL_LUMINANCE
        };

        enum class pixel_type : GLenum
        {
            UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
            UNSIGNED_SHORT_565 = GL_UNSIGNED_SHORT_5_6_5
        };

        struct texture_format
        {
            pixel_format format;
            pixel_type type;
        };

        inline constexpr texture_format R8G8B8A8{ pixel_format::RGBA, pixel_type::UNSIGNED_BYTE };
        inline constexpr texture_format R8G8B8{ pixel_format::RGB, pixel_type::UNSIGNED_BYTE };
        inline constexpr texture_format R5G6B5{ pixel_format::RGB, pixel_type::UNSIGNED_SHORT_565 };
        inline constexpr texture_format LUMINANCE8{ pixel_format::LUMINANCE, pixel_type::UNSIGNED_BYTE };

        [[nodiscard]]
        texture2d_t create_texture2d(size_t width, size_t height, texture_format format, const void* pixels) noexcept;

        struct texture_sampler2D
        {
            uniform_sampler2D_t location;
            sampler2D_value_t value;
        };

        [[nodiscard]]
        texture_sampler2D get_texture_sampler2D(shaders_program_resource program, zstring_view name) noexcept;

        inline void bind(texture_resource2d_t texture, texture_sampler2D sampler) noexcept
        {
            glActiveTexture(narrow_cast<GLenum>(GL_TEXTURE0 + sampler.value));
            bind(texture);
            set(sampler.location, sampler.value);
        }
    }
}
