#include <core/buffer.h>

#include <gl_core/draw.h>

#include <debug/debug.h>

#include <egl_ui/egl_ui_owner.h>

#include <shader/library.h>

#include <utility/png.h>


namespace
{
    struct vertex_shader
    {
        static constexpr auto source = R"(
            attribute vec2 a_position;
            attribute vec2 a_texture;

            varying vec2 v_texture;

            void main()
            {
                gl_Position = vec4(a_position, 0.0, 1.0);
                v_texture = a_texture;
            }
        )"_vert_glsl;

        struct exports
        {
            struct texure_frame
            {
                static constexpr GLfloat radius{ 0.25f };

                static constexpr gl::vertex<gl::vec2f, gl::vec2f> vertices[]
                {
                    { {-radius,  radius}, {0.0f, 0.0f} },
                    { {-radius, -radius}, {0.0f, 1.0f} },
                    { {radius,  radius}, {1.0f, 0.0f} },
                    { {radius, -radius}, {1.0f, 1.0f} }
                };

                static void bind(gl::program_resource p) noexcept
                {
                    static const gl::vertex_buffer vbo{ vertices };
                    vbo.bind(gl::get_attribute_locations(p, "a_position"_zsv, "a_texture"_zsv));
                }

                static void draw() noexcept
                {
                    gl::draw_arrays(gl::draw_mode::triangle_strip, 0, std::size(vertices));
                }
            };
            
            static constexpr texure_frame frame{};

            template<class Fn>
            static constexpr decltype(auto) apply(Fn&& fn) noexcept
            {
                return std::forward<Fn>(fn)(frame);
            }

            template<class>
            struct interface
            {
                static void draw() noexcept
                {
                    frame.draw();
                }
            };
        };
    };

    struct fragment_shader
    {
        static constexpr auto source = R"(
            precision mediump float;

            uniform sampler2D s_texture;
            uniform sampler2D s_mix_texture;
            varying vec2 v_texture;

            void main()
            {                
                vec4 color;
                vec4 mix_color;

                color = texture2D(s_texture, v_texture);
                mix_color = texture2D(s_mix_texture, v_texture);
                gl_FragColor = color * (mix_color + 0.25);
            }
        )"_frag_glsl;

        struct exports
        {
            static constexpr shader_export::sampler texture{ "s_texture"_zsv };
            static constexpr shader_export::sampler mix_texture{ "s_mix_texture"_zsv };

            template<class Fn>
            static constexpr decltype(auto) apply(Fn&& fn) noexcept
            {
                return std::forward<Fn>(fn)(texture, mix_texture);
            }

            template<class Lib>
            struct interface
            {
                static void texture(gl::texture2d_resource tex) noexcept
                {
                    constexpr auto texture_number = shader_common::texture_number<Lib>(exports::texture);
                    gl::store_texture(texture_number, tex);
                }

                static void mix_texture(gl::texture2d_resource tex) noexcept
                {
                    constexpr auto texture_number = shader_common::texture_number<Lib>(exports::mix_texture);
                    gl::store_texture(texture_number, tex);
                }
            };
        };
    };

    template<class... Paths>
    [[nodiscard]] std::array<gl::texture2d, sizeof...(Paths)> png_textures_from_asset(Paths... paths) noexcept
    {
        byte_buffer temp;

        return { png_texture_from_asset(paths, temp)... };
    };
}


int app_main(os::module_handle_t app) noexcept
{
    const auto egl = create_egl_ui(app);
    if (!egl)
    {
        e_debug("create window error: {}", egl_ui::error_code());
        return EXIT_FAILURE;
    }

    const auto [texture, mix_texture] = png_textures_from_asset(_PATH("base.png"), _PATH("mix.png"));

    shader_library<vertex_shader, fragment_shader> shaders{};
    if (!shaders.load())
    {
        e_debug("error loading shaders");
        return EXIT_FAILURE;
    }

    shaders.texture(texture);
    shaders.mix_texture(mix_texture);

    {
        const egl_painting_owner painting_owner{ egl };
        gl::viewport(egl.viewport());
        gl::clear(colors::white_f);
        shaders.draw();
    }

    return ui::run_event_loop(egl);
}




