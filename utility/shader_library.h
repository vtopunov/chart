#pragma once

#include <px/fwd.h>

#include <gl/texture.h>
#include <gl/draw.h>


namespace private_detail_is_safe_conversion_glpx
{
    template<class GL, class PX, class Source>
    [[nodiscard]] constexpr bool is_safe_conversion_glpx_impl(const Source& v) noexcept
    {
        static_assert(std::is_floating_point_v<GL>);
        static_assert(std::is_integral_v<PX>);
        static_assert(std::is_signed_v<PX>);

        constexpr auto gl_digits = numeric_digits_v<GL>;
        constexpr auto px_digits = numeric_digits_v<PX>;
        constexpr auto glpx_digits = std::min(gl_digits, px_digits);
        constexpr auto source_digits = numeric_digits_v<Source>;

        if constexpr (std::is_floating_point_v<Source> || (glpx_digits < source_digits))
        {
            constexpr auto px_max = numeric_max_v<PX>;
            constexpr auto glpx_max = px_max >> (px_digits - glpx_digits);
            constexpr auto glpx_max_source = static_cast<Source>(glpx_max);

            if constexpr (std::is_unsigned_v<Source>)
            {
                return v <= glpx_max_source;
            }
            else
            {
                constexpr auto glpx_lowest = -glpx_max;
                constexpr auto glpx_lowest_source = static_cast<Source>(glpx_lowest);
                return (v >= glpx_lowest_source)
                    && (v <= glpx_max_source);
            }
        }
        else
        {
            return true;
        }
    }
}

template<class Source>
[[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<Source>, bool> is_safe_conversion_glpx(const Source& v) noexcept
{
    return private_detail_is_safe_conversion_glpx::is_safe_conversion_glpx_impl<GLfloat, pxoff_t>(v);
}

template<class T>
[[nodiscard]] constexpr bool is_safe_conversion_glpx(const vec2<T>& v) noexcept
{
    return is_safe_conversion_glpx(v._0)
        && is_safe_conversion_glpx(v._1);
}

template<class Value, class Size>
[[nodiscard]] constexpr bool is_safe_conversion_glpx(const rectangle<Value, Size>& v) noexcept
{
    return is_safe_conversion_glpx(v.position)
        && is_safe_conversion_glpx(v.sizes);
}


template<class T, class U>
[[nodiscard]] constexpr bool update_glpx(T& value, U&& new_value) noexcept
{
    const auto ok = (new_value != value) && is_safe_conversion_glpx(new_value);
    if (ok) [[likely]]
    {
        value = std::forward<U>(new_value);
    }

    return ok;
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<T>, GLfloat> to_glpx(const T& value) noexcept
{
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
    D_ASSERT(is_safe_conversion_glpx(value));
    return static_cast<GLfloat>(value);
    D_WARNING_POP;
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr auto to_glpx(const Vec<T>& v) noexcept -> Vec<decltype(to_glpx(as_vec2(v)._0))>
{
    return
    {
        to_glpx(v._0),
        to_glpx(v._1)
    };
}


struct uniform_vec2glpx
{
    using glsl_uniform_type = gl::uniform_vec2f;
    glsl_uniform_type uniform;

    template<class T>
    void operator () (const vec2<T>& p) const noexcept
    {
        uniform.store(to_glpx(p));
    }

    template<class T>
    void operator () (T p0, T p1) const noexcept
    {
        uniform.store
        (
            to_glpx(p0),
            to_glpx(p1)
        );
    }

    [[nodiscard]]
    static uniform_vec2glpx instance(gl::program_resource program, zstring_view name) noexcept
    {
        return { .uniform{ glsl_uniform_type::instance(program, name) } };
    }
};

struct attribute_frame
{
    static constexpr gl::vec2f vertices[]
    {
        {0.0f, 0.0f},
        {0.0f, 1.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f}
    };

    gl::attribute_location attrib;

    [[nodiscard]]
    static attribute_frame instance(gl::program_resource program, zstring_view name) noexcept
    {
        return { .attrib{ gl::get_attribute_location(program, name) } };
    }

    struct vertex_buffer_user
    {
        static void draw() noexcept
        {
            gl::draw_arrays(gl::draw_mode::triangle_strip, 0, std::size(vertices));
        }
    };

    vertex_buffer_user bind() const noexcept
    {
        static const gl::vertex_buffer vbo{ vertices };
        vbo.bind(attrib);
        return {};
    }

    vertex_buffer_user operator () () const noexcept
    {
        return bind();
    }

    void draw() const noexcept
    {
        bind().draw();
    }
};

namespace vert
{
    struct positioned_frame
    {
        uniform_vec2glpx position{ gl::invaliduniform };
        uniform_vec2glpx size{ gl::invaliduniform };
        uniform_vec2glpx viewport{ gl::invaliduniform };
        attribute_frame  frame{ gl::invalidattribute };

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(position, "u_position"_zsv);
            ser(size, "u_size"_zsv);
            ser(viewport, "u_viewport"_zsv);
            ser(frame, "a_frame"_zsv);
        }
    };

    struct positioned_texture : positioned_frame
    {
        static constexpr auto shader_text = R"(
            precision mediump float;

            uniform vec2 u_position;
            uniform vec2 u_size;
            uniform vec2 u_viewport;

            attribute vec2 a_frame;

            varying vec2 v_texture;

            void main()
            {
                vec2 px_position = (2.0 * (a_frame * u_size + u_position)) / u_viewport;
                gl_Position = vec4(px_position.x - 1.0, 1.0 - px_position.y, 0.0, 1.0);
                v_texture = a_frame;
            }
        )"_glsl;
    };

    struct positioned_rectangle : positioned_frame
    {
        static constexpr auto shader_text = R"(
            precision mediump float;

            uniform vec2 u_position;
            uniform vec2 u_size;
            uniform vec2 u_viewport;

            attribute vec2 a_frame;

            void main()
            {
                vec2 px_position = (2.0 * (a_frame * u_size + u_position)) / u_viewport;
                gl_Position = vec4(px_position.x - 1.0, 1.0 - px_position.y, 0.0, 1.0);
            }
        )"_glsl;
    };
}

namespace frag
{
    struct default_color
    {
        static constexpr auto shader_text = R"(
            precision mediump float;

            uniform vec4 u_color;

            void main()
            {
                gl_FragColor = u_color;
            }
        )"_glsl;

        gl::uniform_vec4f color = gl::invaliduniform;

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(color, "u_color"_zsv);
        }
    };

    struct default_texture
    {
        static constexpr auto shader_text = R"(
            precision mediump float;

            uniform sampler2D s_texture;
            varying vec2 v_texture;

            void main()
            {
                gl_FragColor = texture2D(s_texture, v_texture);
            }
        )"_glsl;

        gl::texture_sampler2D texture = gl::invalidtexsampler;

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(texture, "s_texture"_zsv);
        }
    };

    struct inverted_texture
    {
        static constexpr auto shader_text = R"(
            precision mediump float;

            uniform sampler2D s_texture;
            varying vec2 v_texture;

            void main()
            {
                vec4 tex =  texture2D(s_texture, v_texture);
                gl_FragColor = vec4(1.0 - tex.rgb, tex.a);
            }
        )"_glsl;

        gl::texture_sampler2D texture = gl::invalidtexsampler;

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(texture, "s_texture"_zsv);
        }
    };

    struct luminance_texture_mix_color
    {
        static constexpr auto shader_text = R"(
            precision mediump float;

            uniform vec4 u_color;
            uniform sampler2D s_texture;

            varying vec2 v_texture;

            void main()
            {
                vec4 tex = texture2D(s_texture, v_texture);
                gl_FragColor = vec4(u_color.rgb, u_color.a * tex.r);
            }
        )"_glsl;

        gl::uniform_vec4f color = gl::invaliduniform;
        gl::texture_sampler2D texture = gl::invalidtexsampler;

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(color, "u_color"_zsv);
            ser(texture, "s_texture"_zsv);
        }
    };

    template<class T>
    using decl_color_t = decltype(std::declval<const T&>().color);

    template<class T>
    using decl_texture_t = decltype(std::declval<const T&>().texture);

    template<class T>
    constexpr bool has_color_v = is_detected_v<decl_color_t, T>;

    template<class T>
    using has_texture = is_detected<decl_texture_t, T>;

    template<class T>
    constexpr bool has_texture_v = has_texture<T>::value;
}


template<class VS, class FS>
class shader_library
{
public:
    using vertex_shader_type = VS;
    using fragment_shader_type = FS;

    constexpr explicit operator bool() const noexcept
    {
        return !!program_;
    }

    [[nodiscard]]
    bool build() noexcept
    {
        D_ASSERT(!program_);

        program_ = gl::create_program(vertex_shader_type::shader_text, fragment_shader_type::shader_text);
        if (program_) [[likely]]
        {
            const auto unfiorm_factory = [p = view(program_)]<class T>(T & target, zstring_view name) noexcept
            {
                target = T::instance(p, name);
            };

            serialize(unfiorm_factory);

            return true;
        }

        return false;
    }

    void use() const noexcept
    {
        gl::use(program_);
    }

    constexpr const vertex_shader_type& vert() const noexcept
    {
        D_ASSERT(program_ == gl::current_program());
        return vert_;
    }

    constexpr const fragment_shader_type& frag() const noexcept
    {
        D_ASSERT(program_ == gl::current_program());
        return frag_;
    }

    template<class Serializer>
    constexpr void serialize(Serializer& ser) noexcept
    {
        vert_.serialize(ser);
        frag_.serialize(ser);
    }

private:
    vertex_shader_type vert_{};
    fragment_shader_type frag_{};
    gl::program program_{};
};