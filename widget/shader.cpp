#include "shader.h"

#include <debug/debug.h>

#include <utility/shaders_library.h>

namespace widget
{
    namespace shader
    {
        namespace
        {
            void set_viewport_sizes(const vert::positioned_figure& fig, px::size2d sizes) noexcept
            {
                fig.u_viewport.store(sizes);
            }

            template<class VS, class FS>
            bool initialize(shaders_library<VS, FS>& shaders, px::size2d viewport_sizes) noexcept
            {
                if (!shaders)
                {
                    if (!shaders.build())
                    {
                        e_debug("build shaders program error");
                        return false;
                    }

                    shaders.use();
                    set_viewport_sizes(shaders.vert, viewport_sizes);
                }

                return true;
            }
        }

        namespace colored_rectangle
        {
            namespace
            {
                shaders_library<vert::positioned_rectangle, frag::default_color> shaders{};
            }

            bool initialize(px::size2d viewport_sizes) noexcept
            {
                return shader::initialize(shaders, viewport_sizes);
            }

            void draw(rectangle rc, gl::rgba_colorf_t colorf) noexcept
            {
                shaders.use();
                shaders.vert.u_position.store(rc.position);
                shaders.vert.u_size.store(rc.sizes);
                shaders.frag.u_color.store(colorf);
                shaders.vert.a_frame.draw();
            }
        }

        namespace gray_texture_mix_color
        {
            namespace
            {
                shaders_library<vert::positioned_texture, frag::gray_texture_mix_color> shaders{};
            }

            bool initialize(px::size2d viewport_sizes) noexcept
            {
                return shader::initialize(shaders, viewport_sizes);
            }

            void draw(px::point2d position, gl::texture2d_resources texture, gl::rgba_colorf_t colorf) noexcept
            {
                shaders.use();
                shaders.vert.u_position.store(position);
                shaders.vert.u_size.store(texture.sizes);
                shaders.frag.u_color.store(colorf);
                shaders.frag.s_texture.store(texture);
                shaders.vert.a_frame.draw();
            }
        }
    }
}