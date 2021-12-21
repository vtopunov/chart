#pragma once

#include <gl/texture.h>
#include <gl/draw.h>

namespace vert
{
    using namespace gl_literals;

    struct positioned_texture
    {
        static constexpr auto shader_text = R"(
            uniform vec2 u_position;
            uniform vec2 u_size;
            uniform vec2 u_viewport;

            attribute vec2 a_texture;

            varying vec2 v_texture;

            void main()
            {
                vec2 px_position = (a_texture * u_size + u_position) / u_viewport;
                gl_Position = vec4((2.0 * px_position.x) - 1.0, 1.0 - (2.0 * px_position.y), 0.0, 1.0);
                v_texture = a_texture;
            }
        )"_glsl;

        using vertext_type = gl::vec2f_t;
        gl::vertex_attrib_buffer<vertext_type> vbo;
        gl::uniform_vec2f_t u_position = gl::invaliduniform;
        gl::uniform_vec2f_t u_size = gl::invaliduniform;
        gl::uniform_vec2f_t u_viewport = gl::invaliduniform;

        bool initialize(gl::shaders_program_resource program) noexcept
        {
            u_position = u_position.instance(program, "u_position"_zsv);
            u_size = u_size.instance(program, "u_size"_zsv);
            u_viewport = u_viewport.instance(program, "u_viewport"_zsv);

            {
                constexpr vertext_type vertices[]
                {
                    {0.0f, 0.0f},
                    {0.0f, 1.0f},
                    {1.0f, 0.0f},
                    {1.0f, 1.0f}
                };

                vbo = { vertices, gl::get_attribute_locations(program, "a_texture"_zsv) };
            }

            return !!vbo;
        }
    };
}

namespace frag
{
    using namespace gl_literals;

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

        gl::texture_sampler2D_t s_texture = gl::invalidtexsampler;

        bool initialize(gl::shaders_program_resource program) noexcept
        {
            s_texture = s_texture.instance(program, "s_texture"_zsv);

            return true;
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
                vec4 rgb_gray = texture2D(s_texture, v_texture);
                gl_FragColor = vec4(u_color.rgb, u_color.a * rgb_gray.r);
            }
        )"_glsl;

        gl::uniform_vec4f_t u_color = gl::invaliduniform;
        gl::texture_sampler2D_t s_texture = gl::invalidtexsampler;

        bool initialize(gl::shaders_program_resource program) noexcept
        {
            u_color = u_color.instance(program, "u_color"_zsv);
            s_texture = s_texture.instance(program, "s_texture"_zsv);

            return true;
        }
    };
}


template<class VS, class FS>
struct shaders
{
    VS vert{};
    FS frag{};

    gl::shaders_program_t program{};

    bool build() noexcept
    {
        program.reset();

        if (auto new_program = gl::create_shaders_program(vert.shader_text, frag.shader_text))
        {
            if (vert.initialize(new_program) && frag.initialize(new_program))
            {
                program = std::move(new_program);
                return true;
            }
        }

        return false;
    }

    void use() const noexcept
    {
        gl::use(program);
    }
};


