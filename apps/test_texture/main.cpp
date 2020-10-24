#include <variant>

#include <core/debug.h>

#include <display/event_loop.h>
#include <display/gl/texture.h>
#include <display/gl/draw.h>
#include <display/egl/egl_window.h>

#include <file/file_mmap.h>
#include <image/png.h>

using namespace std::string_view_literals;
using namespace display::gl_literals;
using namespace display;


namespace
{
    gl::texture2d_t png_texture(file::path_string_view_t path) noexcept
    {
        constexpr auto png_format = image::png_format::RGBA8;
        constexpr auto gl_format = gl::R8G8B8A8;

        const auto map_file = file::mmap(path);
        if (!map_file)
        {
            output_debug_string(L"can't mapping file: {}\n", path.as_string_view());
            return {};
        }

        const auto png = image::png_instance();

        const auto accept_png_errno = [] (image::png_errno errc) noexcept
        {
            const auto is_error = errc != image::png_errno::OK;

            if (is_error)
            {
                output_debug_string("png error: {}: {}\n", to_underlying(errc), image::png_error_string(errc).c_str());
            }

            return is_error;
        };

        if (accept_png_errno(image::png_set_buffer(png, map_file)))
        {
            return {};
        }

        const image::png_header png_header{ png };

        size_t size = 0;
        if (accept_png_errno(image::png_decoded_image_size(png, png_format, &size)))
        {
            return {};
        }

        if (!size)
        {
            output_debug_string(L"empty png image: {}\n", path.as_string_view());
            return {};
        }

        uninitialized_dynarray<std::byte> buffer{ size };
        
        if (!buffer)
        {
            output_debug_string("out of memory: size = {}\n", size);
            return {};
        }

        if (accept_png_errno(image::png_decode_image(png, png_format, buffer)))
        {
            return {};
        }

        return gl::create_texture2d(png_header.width(), png_header.height(), gl_format, buffer.data());
    }

    void draw_texture_mix(gl::texture_resource2d_t base_texture, gl::texture_resource2d_t mix_texture) noexcept
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

        static const auto a_position = gl::get_attribute<gl::type_id::vec2f>(shaders, "a_position");
        static const auto a_texture = gl::get_attribute<gl::type_id::vec2f>(shaders, "a_texture");
        static const auto s_base_texture = gl::get_texture_sampler2D(shaders, "s_base_texture");
        static const auto s_mix_texture = gl::get_texture_sampler2D(shaders, "s_mix_texture");

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        gl::use(shaders);

        constexpr GLfloat radius{ 0.25f };

        constexpr GLfloat vertices[] =
        {
            -radius,  radius, 0.0f, 0.0f,
            -radius, -radius, 0.0f, 1.0f,
             radius,  radius, 1.0f, 0.0f,
             radius, -radius, 1.0f, 1.0f
        };


        constexpr auto stride = a_position.tuple_size + a_texture.tuple_size;
        gl::set_pointer(a_position, vertices, stride);
        gl::set_pointer(a_texture, vertices + a_position.tuple_size, stride);

        gl::enable_array(a_position);
        gl::enable_array(a_texture);

        gl::bind(base_texture, s_base_texture);
        gl::bind(mix_texture, s_mix_texture);

        gl::draw_arrays(gl::draw_mode::triangle_strip, 0, std::size(vertices) / stride);

        //{
        //    constexpr GLubyte indices[]{ 0, 1, 3, 0, 3, 2 };
        //    gl::draw_elements(gl::draw_mode::triangles, indices);
        //}
    }

    class main_processor
    {
    public:
        main_processor() = default;

        bool initialize() noexcept
        {
            egl_window_ = egl_window_factory{}.create();
            if (!egl_window_)
            {
                output_debug_string("create window error: window error: {}, egl error: {}\n",
                    display::last_error_code(), eglGetError());
                return false;
            }
            
            base_texture_ = png_texture(_PATH("base.png"));
            if (!base_texture_)
            {
                output_debug_string("create png texture error: {}\n", glGetError());
                return false;
            }

            mix_texture_ = png_texture(_PATH("mix.png"));
            if (!mix_texture_)
            {
                output_debug_string("create png texture error: {}\n", glGetError());
                return false;
            }

            return true;
        }

        void show(int command_show) noexcept
        {
            need_redraw_ = true;
            display::show(egl_window_, command_show);
        }

        void draw() const noexcept
        {
            [[maybe_unused]]
            const auto lock = egl_window_->begin();

            draw_texture_mix(base_texture_, mix_texture_);
        }

        bool operator () (peek_event) const noexcept
        {
            return need_redraw_;
        }
        
        void operator () (idle_event) noexcept
        {
            need_redraw_ = false;
            draw();
        }

        int run() noexcept
        {
            return display::run_event_loop(egl_window_, *this);
        }

    private:
        egl_window_t egl_window_;
        gl::texture2d_t base_texture_;
        gl::texture2d_t mix_texture_;
        bool need_redraw_{ false };
    };
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int command_show)
{
    main_processor processor;

    if (!processor.initialize())
    {
        output_debug_string("initialize fail\n");
        return -1;
    }

    processor.show(command_show);

    return processor.run();
}




