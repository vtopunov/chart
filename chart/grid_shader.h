#pragma once

#include <chart/fwd.h>
#include <widget/shader.h>


namespace chart
{
    namespace shader
    {
        namespace vert
        {
            using grid = ::vert::positioned_rectangle;
        }

        namespace frag
        {
            struct grid
            {
                static constexpr auto eps = 0.1;

                static constexpr auto shader_text = R"(
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
                )"_glsl;

                gl::uniform_vec4f color{ gl::invaliduniform };
                uniform_vec2glpx width{ gl::invaliduniform };
                uniform_vec2glpx begin{ gl::invaliduniform };
                uniform_vec2glpx repeat{ gl::invaliduniform };

                template<class Serializer>
                constexpr void serialize(Serializer& ser) noexcept
                {
                    ser(color, "u_color"_zsv);
                    ser(width, "u_width"_zsv);
                    ser(begin, "u_begin"_zsv);
                    ser(repeat, "u_repeat"_zsv);
                }
            };
        }

        struct grid_user : widget_shader_user<grid_user, vert::grid, frag::grid>
        {
            using widget_shader_user::frag;

            const grid_user& width(pxvec2d new_width) const noexcept
            {
                frag().width(new_width);
                return *this;
            }

            const grid_user& begin(real_vec2 new_width) const noexcept
            {
                frag().begin(new_width);
                return *this;
            }

            const grid_user& repeat(real_vec2 new_width) const noexcept
            {
                frag().repeat(new_width);
                return *this;
            }
        };

        using grid = widget_shader_library<vert::grid, frag::grid, grid_user>;
    }
}
