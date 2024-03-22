#pragma once

#include <core/size2d.h>

#include <px/pixspan.h>

#include <gl/shader.h>


namespace gl
{
    constexpr size_t default_alignment{ 4_uz };
    static_assert(px::default_alignment == gl::default_alignment);

    enum class texture_target : GLenum
    {
        texture_2d = GL_TEXTURE_2D
    };

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

    inline void bind_texture(texture_target target, texture_resource texture) noexcept
    {
        glBindTexture(to_underlying(target), texture.d);
    }

    template<texture_target Target>
    struct specialized_texture_resource : texture_resource
    {
        static constexpr auto target = Target;
    };

    template<texture_target Target>
    inline void bind_texture(specialized_texture_resource<Target> texture) noexcept
    {
        bind_texture(texture.target, texture);
    }

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

    static_assert(std::is_same_v<null_t<texture2d_resources>, texture2d_resources::null_type>);
    static_assert(std::is_same_v<view_t<texture2d_resources>, const texture2d_resources::view_type>);

    template<class T>
    using unique_texture = unique_resource<T, texture_resource_deleter>;

    using unique_texture2d_resource = unique_resource<texture2d_resource, texture_resource_deleter>;

    using texture2d = unique_texture<texture2d_resources>;
    static_assert(std::is_same_v<view_t<texture2d>, const texture2d::view_type>);

    [[nodiscard]]
    texture2d create_texture2d() noexcept;

    [[nodiscard]]
    texture2d create_texture2d(pxsize2d sizes, texture_format format, const void* pixels) noexcept;

    pxsize2d write(texture2d_resource texture, pxsize2d sizes, texture_format format, const void* pixels) noexcept;

    [[nodiscard]]
    inline texture2d sizes(texture2d tex, pxsize2d sizes) noexcept
    {
        return
        {
            resource_construct,
            tex.release(),
            sizes
        };
    }

    [[nodiscard]]
    inline texture2d sizes(texture2d tex, pxsize_t w, pxsize_t h) noexcept
    {
        return sizes(std::move(tex), pxsize2d{ w, h });
    }

    [[nodiscard]]
    inline texture2d image(texture2d texture, pxsize2d pxsizes, texture_format format, const void* pixels) noexcept
    {
        const auto wpxsizes = write(texture, pxsizes, format, pixels);
        return sizes(std::move(texture), wpxsizes);
    }

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
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<T>, texture2d> create_texture2d(pixspan<T> img) noexcept
    {
        return create_texture2d(img.sizes(), texpix_format_v<T>, img.cdata());
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<T>, texture2d> image(texture2d texture, pixspan<T> img) noexcept
    {
        return image(std::move(texture), img.sizes(), texpix_format_v<T>, img.cdata());
    }

    template<class T>
    std::enable_if_t<texpix_enabled_v<T>, pxsize2d> write(texture2d_resource texture, pixspan<T> img) noexcept
    {
        return write(texture, img.sizes(), texpix_format_v<T>, img.cdata());
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<T>, bool> update(texture2d& texture_ref, pixspan<T> img) noexcept
    {
        if (texture_ref)
        {
            texture_ref = image(std::move(texture_ref), img);
        }
        else
        {
            texture_ref = create_texture2d(img);
        }

        return sizes(texture_ref) == img.sizes();
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<T>, bool> update(unique_texture2d_resource& texture_ref, pixspan<T> img) noexcept
    {
        constexpr auto set_new_texture = [] (unique_texture2d_resource& texture_ref, texture2d&& new_texture) noexcept
        {
            const auto tex_d = new_texture.release();
            D_UNUSED(texture_ref.release(static_cast<texture2d_resource>(tex_d)));
            return tex_d.sizes;
        };

        pxsize2d wsizes{};

        if (texture_ref)
        {
            wsizes = write(texture_ref, img);
        }
        else
        {
            wsizes = set_new_texture(texture_ref, gl::create_texture2d(img));
        }

        return img.sizes() == wsizes;
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
    std::underlying_type_t<uniform_location> get_sampler_number(program_resource program, uniform_location location) noexcept;

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
            glActiveTexture(narrow<GLenum>(GL_TEXTURE0 + value));
            bind_texture(target, texture);
            sampler.store(value);
        }

        [[nodiscard]]
        static texture_sampler instance(program_resource program, zstring_view name) noexcept
        {
            const auto sampler_location = uniform_sampler_type::instance(program, name);

            return
            {
                sampler_location,
                narrow<sampler_value_type>(get_sampler_number(program, sampler_location.location))
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
