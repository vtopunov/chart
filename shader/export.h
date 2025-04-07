#pragma once

#include <gl_core/texture.h>
#include <gl_core/draw.h>


namespace shader_export
{
    struct uniform_base
    {
        zstring_view name;
    };

    template<gl::glsl_typeid id>
    struct typed_uniform_base : uniform_base
    {
        using gl_uniform_type = gl::uniform<id>;

        gl::uniform_location location(gl::program_resource p) const noexcept
        {
            return gl_uniform_type::instance(p, name).location;
        }

        template<class... Types>
        static auto store(gl::uniform_location loc, const Types&... values) noexcept -> decltype(gl_uniform_type{loc}.store(values...))
        {
            return gl_uniform_type{loc}.store(values...);
        }
    };

    using pxfvec = typed_uniform_base<gl::glsl_typeid::vec2f>;
    using color = typed_uniform_base<gl::glsl_typeid::vec4f>;
    using sampler = typed_uniform_base<gl::glsl_typeid::sampler2D>;

    struct frame
    {
        static constexpr gl::vec2f vertices[]
        {
            {0.0f, 0.0f},
            {0.0f, 1.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f}
        };

        zstring_view name;

        static void draw() noexcept
        {
            gl::draw_arrays(gl::draw_mode::triangle_strip, 0, std::size(vertices));
        }

        void bind(gl::program_resource p) const noexcept
        {
            static const gl::vertex_buffer vbo{ vertices };
            vbo.bind(gl::get_attribute_location(p, name));
        }
    };
}
