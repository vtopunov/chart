#pragma once

#include <px/pixspan.h>

#include <gl_core/shader.h>


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

    inline void store_texture(GLenum num, texture2d_resource texture) noexcept
    {
        glActiveTexture(num);
        bind_texture(texture);
    }

    struct texture2d_resources : texture2d_resource
    {
        using base_resource_type = texture2d_resource;

        using view_type = base_resource_type;

        struct null_type : null_t<base_resource_type>
        {
            constexpr operator texture2d_resources () const noexcept
            {
                return { static_cast<base_resource_type>(*this), px::no_sizes };
            }
        };

        pxsizes sizes;
    };

    static_assert(std::is_same_v<decl_null_type_t<texture2d_resources>, texture2d_resources::null_type>);
    static_assert(std::is_same_v<decl_view_type_t<texture2d_resources>, texture2d_resources::view_type>);

    template<class T>
    struct unique_texture : unique_resource<T, texture_resource_deleter>
    {
        using unique_resource<T, texture_resource_deleter>::unique_resource;
    };

    using unique_texture2d_resource = unique_resource<texture2d_resource, texture_resource_deleter>;

    template<>
    struct unique_texture<texture2d_resources> : unique_resource<texture2d_resources, texture_resource_deleter>
    {
        using unique_resource::unique_resource;

        constexpr void hide() noexcept
        {
            as_mutable(r().sizes) = px::no_sizes;
        }

        [[nodiscard]]
        constexpr pxsizes sizes() const noexcept
        {
            return r().sizes;
        }

        [[nodiscard]]
        constexpr npx_t width() const noexcept
        {
            return r().sizes.width();
        }

        [[nodiscard]]
        constexpr npx_t height() const noexcept
        {
            return r().sizes.height();
        }
    };

    using texture2d = unique_texture<texture2d_resources>;

    [[nodiscard]]
    texture2d create_texture2d() noexcept;

    [[nodiscard]]
    texture2d create_texture2d(pxsizes sizes, texture_format format, const void* pixels) noexcept;

    pxsizes write(texture2d_resource texture, pxsizes sizes, texture_format format, const void* pixels) noexcept;

    [[nodiscard]]
    inline texture2d image(texture2d texture, pxsizes pxsizes, texture_format format, const void* pixels) noexcept
    {
        as_mutable(texture.r().sizes) = write(texture, pxsizes, format, pixels);
        return texture;
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

    template<class Image>
    constexpr bool texpix_enabled_v = texpix_traits<sizeof(decl_std_data_value_t<Image>)>::enabled;

    template<class Image>
    constexpr auto texpix_format_v = texpix_traits<sizeof(decl_std_data_value_t<Image>)>::format;

    template<class Image>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<Image>, texture2d> create_texture2d(const Image& img) noexcept
    {
        return create_texture2d(::sizes(img), texpix_format_v<Image>, as_const_pointer(std::data(img)));
    }

    template<class Image>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<Image>, texture2d> image(texture2d texture, const Image& img) noexcept
    {
        return image(std::move(texture), ::sizes(img), texpix_format_v<Image>, as_const_pointer(std::data(img)));
    }

    template<class Image>
    std::enable_if_t<texpix_enabled_v<Image>, pxsizes> write(texture2d_resource texture, const Image& img) noexcept
    {
        return write(texture, ::sizes(img), texpix_format_v<Image>, as_const_pointer(std::data(img)));
    }

    template<class Image>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<Image>, bool> update(texture2d& texture_ref, const Image& img) noexcept
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

    template<class Image>
    [[nodiscard]] std::enable_if_t<texpix_enabled_v<Image>, bool> update(unique_texture2d_resource& texture_ref, const Image& img) noexcept
    {
        constexpr auto set_new_texture = [] (unique_texture2d_resource& texture_ref, texture2d&& new_texture) noexcept
        {
            const auto tex_d = new_texture.release();
            D_UNUSED(texture_ref.release(static_cast<texture2d_resource>(tex_d)));
            return tex_d.sizes;
        };

        pxsizes wsizes{};

        if (texture_ref)
        {
            wsizes = write(texture_ref, img);
        }
        else
        {
            wsizes = set_new_texture(texture_ref, gl::create_texture2d(img));
        }

        return ::sizes(img) == wsizes;
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
            store_texture(narrow<GLenum>(GL_TEXTURE0 + value), texture);
            sampler.store(value);
        }

        void operator () (specialized_texture_resource<target> texture) const noexcept
        {
            store(texture);
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
