#pragma once

#include <core/size2d.h>

#include <px/pixmap.h>

#include <gl/shader.h>

namespace gl
{
    constexpr size_t default_alignment{ 4_uz };
    static_assert(px::default_alignment == gl::default_alignment);

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
        void operator () (texture_resource texture) const noexcept;
    };

    enum class texture_target : GLenum
    {
        texture_2d = GL_TEXTURE_2D
    };


    inline void bind_texture(texture_target target, texture_resource texture) noexcept
    {
        glBindTexture(to_underlying(target), texture.d);
    }

    template<texture_target Target>
    struct specialized_texture_resource : texture_resource
    {
        static constexpr auto target = Target;

        void bind() const noexcept
        {
            bind_texture(target, *this);
        }
    };

    using texture2d_resource = specialized_texture_resource<texture_target::texture_2d>;

    struct texture2d_resources : texture2d_resource
    {
        using base_resource_type = texture2d_resource;

        using view_type = base_resource_type;

        struct null_type : null_t<base_resource_type>
        {
            constexpr operator texture2d_resources () const noexcept
            {
                return { static_cast<base_resource_type>(*this), {} };
            }
        };

        pxsize2d sizes;
    };

    using texture2d = unique_resource<texture2d_resources, texture_resource_deleter>;

    [[nodiscard]]
    constexpr pxsize2d sizes(const texture2d_resources& tex) noexcept
    {
        return tex.sizes;
    }

    [[nodiscard]]
    constexpr pxside_t width(const texture2d_resources& tex) noexcept
    {
        return tex.sizes.width();
    }

    [[nodiscard]]
    constexpr pxside_t height(const texture2d_resources& tex) noexcept
    {
        return tex.sizes.height();
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

    constexpr texture_format R8G8B8A8{ pixel_format::RGBA, pixel_type::UNSIGNED_BYTE };
    constexpr texture_format R8G8B8{ pixel_format::RGB, pixel_type::UNSIGNED_BYTE };
    constexpr texture_format R5G6B5{ pixel_format::RGB, pixel_type::UNSIGNED_SHORT_565 };
    constexpr texture_format LUMINANCE8{ pixel_format::LUMINANCE, pixel_type::UNSIGNED_BYTE };

    [[nodiscard]]
    texture2d create_texture2d(pxsize2d sizes, texture_format format, const void* pixels) noexcept;

    template<size_t PxSize>
    struct texpix_traits 
    {
        static constexpr bool enabled{ false };
    };

    template<>
    struct texpix_traits<1_uz>
    {
        static constexpr bool enabled{ true };
        static constexpr texture_format format{ LUMINANCE8 };
    };

    template<>
    struct texpix_traits<4_uz>
    {
        static constexpr bool enabled{ true };
        static constexpr texture_format format{ R8G8B8A8 };
    };

    template<class T>
    constexpr bool texpix_enabled_v = texpix_traits<sizeof(T)>::enabled;

    template<class T>
    constexpr auto texpix_format_v = texpix_traits<sizeof(T)>::format;

    template<class T> 
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<T>, texture2d> create_texture2d(pxsize2d sizes, const T* pixels) noexcept
    {
        return create_texture2d(sizes, texpix_format_v<T>, pixels);
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<T>, texture2d> create_texture2d(pixspan<T> image) noexcept
    {
        return create_texture2d(image.sizes(), image.data());
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<T>, texture2d> create_texture2d(const pixmap<T>& image) noexcept
    {
        return create_texture2d(pixspan{image});
    }

    template<texture_target target>
    struct select_glsl_sampler_typeid
    {};

    template<>
    struct select_glsl_sampler_typeid<texture_target::texture_2d> : glsl_typeid_constant<glsl_typeid::sampler2D>
    {};

    template<texture_target target>
    constexpr auto glsl_sampler_typeid_v = select_glsl_sampler_typeid<target>::value;

    [[nodiscard]]
    std::underlying_type_t<uniform_location> get_sampler_number(shaders_program_resource program, uniform_location location) noexcept;

    struct null_tex_sampler;

    template<texture_target target>
    struct texture_sampler
    {
        using null_type = null_tex_sampler;

        static constexpr auto sampler_typeid = glsl_sampler_typeid_v<target>;
        using uniform_sampler_type = uniform<sampler_typeid>;
        using sampler_value_type = glsl_tuple_element_type_t<sampler_typeid>;
        static_assert(std::is_same_v<sampler_value_type, glsl_view_t<sampler_typeid>>);

        uniform_sampler_type sampler;
        sampler_value_type value;

        void store(specialized_texture_resource<target> texture) const noexcept
        {
            glActiveTexture(narrow_cast<GLenum>(GL_TEXTURE0 + value));
            texture.bind();
            sampler.store(value);
        }

        [[nodiscard]]
        static texture_sampler instance(shaders_program_resource program, zstring_view name) noexcept
        {
            const auto sampler_location = uniform_sampler_type::instance(program, name);

            return
            {
                sampler_location,
                narrow_cast<sampler_value_type>(get_sampler_number(program, sampler_location.location))
            };
        }
    };

    struct null_tex_sampler
    {
        template<texture_target target>
        [[nodiscard]] constexpr operator texture_sampler<target>() const noexcept
        {
            return { invaliduniform, {} };
        }
    };

    using nulltexsampler_t = null_tex_sampler;
    constexpr nulltexsampler_t invalidtexsampler{};

    using texture_sampler2D = texture_sampler<texture_target::texture_2d>;
    static_assert(std::is_same_v<nulltexsampler_t, null_t<texture_sampler2D> >);
}

using gl::sizes;
using gl::width;
using gl::height;
