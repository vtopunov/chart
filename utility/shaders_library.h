#pragma once

#include <px/fwd.h>

#include <gl/texture.h>
#include <gl/draw.h>


namespace px
{
    struct uniform_vec2
    {
        using glsl_uniform_type = gl::uniform_vec2f;
        static constexpr auto type_id = glsl_uniform_type::type_id;
        using value_tuple_type = gl::glsl_type_t<type_id>;
        using value_type = typename value_tuple_type::value_type;

        glsl_uniform_type uniform;

        template<class T>
        void store(::vec2<T> p) const noexcept
        {
            if constexpr (std::is_same_v<value_tuple_type, ::vec2<T>>)
            {
                uniform.store(std::move(p));
            }
            else
            {
                uniform.store(narrow2d<value_tuple_type>(std::move(p)));
            }
        }

        template<class T>
        void store(T p0, T p1) const noexcept
        {
            if constexpr (std::is_same_v<value_type, std::remove_cvref_t<T>>)
            {
                uniform.store(std::move(p0), std::move(p1));
            }
            else
            {
                uniform.store
                (
                    narrow<value_type>(std::move(p0)),
                    narrow<value_type>(std::move(p1))
                );
            }
        }

        [[nodiscard]]
        static uniform_vec2 instance(gl::shaders_program_resource program, zstring_view name) noexcept
        {
            return { .uniform{ glsl_uniform_type::instance(program, name) } };
        }
    };
}


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
    static attribute_frame instance(gl::shaders_program_resource program, zstring_view name) noexcept
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

    void draw() const noexcept
    {
        bind().draw();
    }
};

namespace vert
{
    struct positioned_frame
    {
        px::uniform_vec2 u_position{ gl::invaliduniform };
        px::uniform_vec2 u_size{ gl::invaliduniform };
        px::uniform_vec2 u_viewport{ gl::invaliduniform };
        attribute_frame  a_frame{ gl::invalidattribute };

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(u_position, "u_position"_zsv);
            ser(u_size, "u_size"_zsv);
            ser(u_viewport, "u_viewport"_zsv);
            ser(a_frame, "a_frame"_zsv);
        }
    };

    struct positioned_texture : positioned_frame
    {
        static constexpr auto shader_text = R"(
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

        gl::uniform_vec4f u_color = gl::invaliduniform;

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(u_color, "u_color"_zsv);
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

        gl::texture_sampler2D s_texture = gl::invalidtexsampler;

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(s_texture, "s_texture"_zsv);
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

        gl::texture_sampler2D s_texture = gl::invalidtexsampler;

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(s_texture, "s_texture"_zsv);
        }
    };

    struct gray_texture_mix_color
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

        gl::uniform_vec4f u_color = gl::invaliduniform;
        gl::texture_sampler2D s_texture = gl::invalidtexsampler;

        template<class Serializer>
        constexpr void serialize(Serializer& ser) noexcept
        {
            ser(u_color, "u_color"_zsv);
            ser(s_texture, "s_texture"_zsv);
        }
    };
}


template<class VS, class FS>
struct shaders_library
{
    VS vert{};
    FS frag{};

    gl::shaders_program program{};

    constexpr explicit operator bool() const noexcept
    {
        return !!program;
    }

    bool build() noexcept
    {
        D_ASSERT(!program);

        program = gl::create_shaders_program(vert.shader_text, frag.shader_text);
        if (program) [[likely]]
        {
            const auto unfiorm_factory = [p = view(program)]<class T>(T& target, zstring_view name) noexcept
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
        gl::use(program);
    }

    template<class Serializer>
    constexpr void serialize(Serializer& ser) noexcept
    {
        vert.serialize(ser);
        frag.serialize(ser);
    }
};