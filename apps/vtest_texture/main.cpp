#include <gl/draw.h>

#include <debug/debug.h>

#include <egl/event_loop.h>

#include <utility/png.h>

namespace
{
    void draw_texture_mix(gl::texture2d_resource base_texture, gl::texture2d_resource mix_texture) noexcept
    {
        static const auto shaders = gl::create_shaders_program
        (
            R"(
                attribute vec2 a_position;
                attribute vec2 a_texture;

                varying vec2 v_texture;

                void main()
                {
                    gl_Position = vec4(a_position, 0.0, 1.0);
                    v_texture = a_texture;
                }
            )"_glsl,
            R"(
                precision mediump float;

                uniform sampler2D s_base_texture;
                uniform sampler2D s_mix_texture;
                varying vec2 v_texture;

                void main()
                {                
                    vec4 base_color;
                    vec4 mix_color;

                    base_color = texture2D(s_base_texture, v_texture);
                    mix_color = texture2D(s_mix_texture, v_texture);
                    gl_FragColor = base_color * (mix_color + 0.25);
                }
           )"_glsl
        );

        static const auto attributes = gl::get_attribute_locations(shaders, "a_position"_zsv, "a_texture"_zsv);
        static const auto s_base_texture = gl::texture_sampler2D::instance(shaders, "s_base_texture"_zsv);
        static const auto s_mix_texture = gl::texture_sampler2D::instance(shaders, "s_mix_texture"_zsv);

        gl::clear(gl::colors::white_f);

        gl::use(shaders);

        constexpr GLfloat radius{ 0.25f };
        
        constexpr gl::vertex<gl::vec2f, gl::vec2f> vertices[]
        {
            { {-radius,  radius}, {0.0f, 0.0f} },
            { {-radius, -radius}, {0.0f, 1.0f} },
            { {radius,  radius}, {1.0f, 0.0f} },
            { {radius, -radius}, {1.0f, 1.0f} }
        };

        static const gl::vertex_buffer vbo{ vertices };

        s_base_texture.store(base_texture);
        s_mix_texture.store(mix_texture);

        vbo.bind(attributes).draw(gl::draw_mode::triangle_strip);

        // {
        //   constexpr GLubyte indices[]{ 0, 2, 1, 1, 2, 3 };
        //   gl::draw_elements(gl::draw_mode::triangles, indices);
        // }
    }

    template<class... Paths>
    [[nodiscard]] std::array<gl::texture2d, sizeof...(Paths)>  png_textures_from_file(Paths... paths) noexcept
    {
        buffer_t temp;

        return { png_texture_from_file(paths, temp)... };
    };
}


int app_main(os::module_handle_t app) noexcept
{
    const auto egl = egl::instance(app);
    if (!egl)
    {
        e_debug("create window error: window error: {}, egl error: {}\n",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    const auto [base_texture, mix_texture] = png_textures_from_file(_PATH("base.png"), _PATH("mix.png"));

    {
        egl::painting_owner painting_lock{ egl };
        draw_texture_mix(base_texture, mix_texture);
    }

    return egl::run(egl);
}




