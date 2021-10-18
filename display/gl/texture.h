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

        template<texture_target target>
        struct select_glsl_sampler_typeid
        {};

        template<>
        struct select_glsl_sampler_typeid<texture_target::texture_2d> : glsl_typeid_constant<glsl_typeid::sampler2D>
        {};

        template<texture_target target>
        inline constexpr auto glsl_sampler_typeid_v = select_glsl_sampler_typeid<target>::value;

        template<texture_target target>
        using uniform_sampler_t = uniform<glsl_sampler_typeid_v<target>>;

        template<texture_target target>
        using sampler_value_t = glsl_tuple_element_type_t<glsl_sampler_typeid_v<target>>;

        template<texture_target target>
        struct texture_sampler
        {
            uniform_sampler_t<target> location;
            sampler_value_t<target> value;
        };

        [[nodiscard]]
        std::underlying_type_t<uniform_location> get_sampler_number(shaders_program_resource program, uniform_location location) noexcept;

        template<texture_target target> [[nodiscard]]
        texture_sampler<target> get_texture_sampler(shaders_program_resource program, zstring_view name) noexcept
        {
            const auto sampler_uniform_location = get_uniform_location(program, name);

            const texture_sampler<target> sampler
            {
                .location{ sampler_uniform_location  },
                .value{ narrow_cast<sampler_value_t<target>>( get_sampler_number(program, sampler_uniform_location) ) }
            };

            D_ASSERT(sampler.location.test(program, name));

            return sampler;
        }

        using texture_sampler2D_t = texture_sampler<texture_target::texture_2d>;

        [[nodiscard]]
        inline texture_sampler2D_t get_texture_sampler2D(shaders_program_resource program, zstring_view name) noexcept
        {
            return get_texture_sampler<texture_target::texture_2d>(program, name);
        }

        template<texture_target Target>
        struct specialized_texture_resource : texture_resource
        {
            static constexpr auto target = Target;

            void bind() const noexcept
            {
                glBindTexture(to_underlying(target), d);
            }

            void bind(texture_sampler<target> sampler) const noexcept
            {
                glActiveTexture(narrow_cast<GLenum>(GL_TEXTURE0 + sampler.value));
                bind();
                set(sampler.location, sampler.value);
            }
        };

        using texture_resource2d_t = specialized_texture_resource<texture_target::texture_2d>;

        using texture2d_t = unique_resource<texture_resource2d_t, texture_resource_deleter>;

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
    }
}
