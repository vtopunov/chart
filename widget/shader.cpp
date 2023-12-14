#include "shader.h"

namespace widget
{
    namespace shader
    {
        namespace
        {
            void initialize_viewport(const vert::positioned_frame& vert, pxsize2d sizes) noexcept
            {
                vert.u_viewport.store(sizes);
            }

            template<class VS, class FS>
            void initialize_lib_uniforms(const shader_library<VS, FS>& lib, pxsize2d viewport_sizes) noexcept
            {
                lib.use();
                initialize_viewport(lib.vert, viewport_sizes);
            }

            template<class VS, class FS>
            [[nodiscard]] bool initialize_lib(shader_library<VS, FS>& lib, pxsize2d viewport_sizes) noexcept
            {
                if (lib || lib.build()) [[likely]]
                {
                    initialize_lib_uniforms(lib, viewport_sizes);
                    return true;
                }

                return false;
            }
        }

        bool gray_texture_mix_color::operator()(viewport_size2d viewport) noexcept
        {
            return initialize_lib(lib_, viewport);
        }

        bool colored_rectangle::operator()(viewport_size2d viewport) noexcept
        {
            return initialize_lib(lib_, viewport);
        }
    }
}
