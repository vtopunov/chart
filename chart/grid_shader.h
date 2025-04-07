#pragma once

#include <chart/fwd.h>
#include <widget/shader.h>


namespace chart
{
    namespace shader
    {
        namespace vert
        {
            using grid = shader_embed::vert::positioned_rectangle;
        }

        namespace frag
        {
            struct grid
            {
                static constexpr auto eps = 0.1;

                static constexpr auto source = R"(
                    precision mediump float;

                    uniform vec2 u_position;
                    uniform vec2 u_size;
                    uniform vec2 u_viewport;
                    uniform vec4 u_color;
                    uniform vec2 u_width;
                    uniform vec2 u_begin;
                    uniform vec2 u_repeat;                 

                    const float eps = 0.1;

                    bvec2 grid(vec2 coord)
                    {
                        vec2 t = mod(floor(coord), u_repeat) - u_width;
                        return bvec2(t.x < eps, t.y < eps);
                    }

                    void main()
                    {
                        vec2 offset = vec2(u_position.x, u_viewport.y - u_size.y - u_position.y) + u_begin - 0.5 * u_width;
                        vec2 current = gl_FragCoord.xy - offset;
                        vec2 out_of_gridline = current + u_width;
                        bvec2 b_grid = grid(current);
                        bvec2 b_out_of_grid = grid(out_of_gridline);
                        if((b_grid.x && !b_out_of_grid.x) || (b_grid.y && !b_out_of_grid.y))
                        {
                            gl_FragColor = u_color;
                        }
                        else
                        {
                            gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
                        }
                    }
                )"_frag_glsl;

                struct ex_exports
                {
                    static constexpr shader_export::pxfvec width{ "u_width"_zsv };
                    static constexpr shader_export::pxfvec begin{ "u_begin"_zsv };
                    static constexpr shader_export::pxfvec repeat{ "u_repeat"_zsv };

                    template<class Fn>
                    static constexpr decltype(auto) apply(Fn&& fn) noexcept
                    {
                        return std::forward<Fn>(fn)(width, begin, repeat);
                    }

                    template<class Lib>
                    struct interface
                    {
                        static constexpr shader_common::import_engine<Lib> import{};

                        void width(pxvec new_width) const noexcept
                        {
                            constexpr auto store = import(ex_exports::width);
                            store(*this, new_width);
                        }

                        void begin(px::real_vec2 new_begin) const noexcept
                        {
                            constexpr auto store = import(ex_exports::begin);
                            store(*this, new_begin);
                        }

                        void repeat(px::real_vec2 new_repeat) const noexcept
                        {
                            constexpr auto store = import(ex_exports::repeat);
                            store(*this, new_repeat);
                        }
                    };
                };

                using exports = types_cat_t<shader_embed::frag::default_color::exports, ex_exports>;
            };
        }

        using grid = shader_library<vert::grid, frag::grid>;
    }
}
