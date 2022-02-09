#pragma once

#include <gl/texture.h>
#include <gl/draw.h>
#include <gl/color.h>

namespace px
{
    struct uniform_point2d
    {
        using glsl_uniform_type = gl::uniform_vec2f;

        glsl_uniform_type uniform;

        void store(px::point2d p) const noexcept
        {
            uniform.store(narrow2d_cast<gl::vec2f>(p));
        }

        static uniform_point2d instance(gl::shaders_program_resource program, zstring_view name) noexcept
        {
            return { glsl_uniform_type::instance(program, name) };
        }
    };

    struct uniform_size2d
    {
        using glsl_uniform_type = gl::uniform_vec2f;

        glsl_uniform_type uniform;

        void store(px::size2d sizes) const noexcept
        {
            uniform.store(narrow2d_cast<gl::vec2f>(sizes));
        }

        static uniform_size2d instance(gl::shaders_program_resource program, zstring_view name) noexcept
        {
            return { .uniform{ glsl_uniform_type::instance(program, name) } };
        }
    };
}


namespace figures
{
    struct frame_attribute
    {
        gl::attribute_location attrib;

        static frame_attribute instance(gl::shaders_program_resource program, zstring_view name) noexcept
        {
            return { .attrib{ gl::get_attribute_location(program, name) } };
        }

        struct user
        {
            gl::vertex_buffer_user vb_user;

            void draw() const
            {
                vb_user.draw(gl::draw_mode::triangle_strip);
            }
        };

        user bind() const noexcept
        {
            constexpr gl::vec2f vertices[]
            {
                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 0.0f},
                {1.0f, 1.0f}
            };

            static const gl::vertex_buffer vbo{ vertices };

            return { .vb_user{ vbo.bind(attrib) } };
        }


        void draw() const noexcept
        {
            bind().draw();
        }
    };
}


struct shader_initializer
{
    gl::shaders_program_resource program;

    template<class T>
    void operator () (T& target, zstring_view name) const noexcept
    {
        target = T::instance(program, name);
    }
};


namespace vert
{
    struct positioned_texture
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

        px::uniform_point2d      u_position{ gl::invaliduniform };
        px::uniform_size2d       u_size{ gl::invaliduniform };
        px::uniform_size2d       u_viewport{ gl::invaliduniform };
        figures::frame_attribute a_frame{ gl::invalidattribute };

        bool initialize(shader_initializer ini) noexcept
        {
            ini(u_position, "u_position"_zsv);
            ini(u_size, "u_size"_zsv);
            ini(u_viewport, "u_viewport"_zsv);
            ini(a_frame, "a_frame"_zsv);
            return true;
        }
    };

    struct positioned_rect
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

        px::uniform_point2d      u_position{ gl::invaliduniform };
        px::uniform_size2d       u_size{ gl::invaliduniform };
        px::uniform_size2d       u_viewport{ gl::invaliduniform };
        figures::frame_attribute a_frame{ gl::invalidattribute };

        bool initialize(shader_initializer ini) noexcept
        {
            ini(u_position, "u_position"_zsv);
            ini(u_size, "u_size"_zsv);
            ini(u_viewport, "u_viewport"_zsv);
            ini(a_frame, "a_frame"_zsv);
            return true;
        }
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

        bool initialize(shader_initializer ini) noexcept
        {
            ini(u_color, "u_color"_zsv);
            return true;
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

        bool initialize(shader_initializer ini) noexcept
        {
            ini(s_texture, "s_texture"_zsv);
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

        gl::uniform_vec4f u_color = gl::invaliduniform;
        gl::texture_sampler2D s_texture = gl::invalidtexsampler;

        bool initialize(shader_initializer ini) noexcept
        {
            ini(u_color, "u_color"_zsv);
            ini(s_texture, "s_texture"_zsv);
            return true;
        }
    };
}


template<class VS, class FS>
struct shaders_library
{
    VS vert{};
    FS frag{};

    gl::shaders_program program{};


    bool build() noexcept
    {
        if (program)
        {
            return true;
        }

        if (auto new_program = gl::create_shaders_program(vert.shader_text, frag.shader_text))
        {
            const shader_initializer ini
            {
                view(new_program)
            };

            if (vert.initialize(ini) && frag.initialize(ini))
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


