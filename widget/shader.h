#pragma once

#include <egl_ui/viewport_size2d.h>

#include <utility/shader_library.h>


namespace widget
{
    namespace shader
    {
        namespace private_detail_shader
        {
#if D_IS_DEBUG
            template<gl::glsl_typeid type_id>
            [[nodiscard]] constexpr bool is_initialized(const gl::uniform<type_id>& u) noexcept
            {
                return u.__debug_is_stored();
            }

            [[nodiscard]] constexpr bool is_initialized(const uniform_vec2glpx& vec) noexcept
            {
                return is_initialized(vec.uniform);
            }

            [[nodiscard]] constexpr bool is_initialized(const gl::texture_sampler2D& sampler) noexcept
            {
                return is_initialized(sampler.sampler);
            }

            [[nodiscard]] constexpr bool is_initialized(attribute_frame) noexcept
            {
                return true;
            }

            template<class VS, class FS>
            [[nodiscard]] constexpr bool all_is_initialized(const shader_library<VS, FS>& lib) noexcept
            {
                bool result{ true };

                const auto tester = [&result] (const auto& u, zstring_view) noexcept
                {
                    result = result && is_initialized(u);
                    D_ASSERT(result);
                };

                as_mutable(lib).serialize(tester);

                return result;
            }
#endif

            inline void draw(const vert::positioned_frame& vert) noexcept
            {
                vert.a_frame.draw();
            }

            template<class VS, class FS>
            void draw(const shader_library<VS, FS>& lib) noexcept
            {
                D_ASSERT(all_is_initialized(lib));
                draw(lib.vert);
            }
        }

        class gray_texture_mix_color
        {
        public:
            bool operator () (viewport_size2d viewport) noexcept;

            void draw(pxpoint2d position, gl::texture2d_resources texture, gl::rgba_colorf_t colorf) const noexcept
            {
                lib_.use();
                lib_.frag.u_color.store(colorf);
                lib_.frag.s_texture.store(texture);
                lib_.vert.u_position.store(position);
                lib_.vert.u_size.store(sizes(texture));
                private_detail_shader::draw(lib_);
            }

            template<class Fn>
            decltype(auto) apply(Fn fn) noexcept
            {
                return fn();
            }

        private:
            shader_library<vert::positioned_texture, frag::gray_texture_mix_color> lib_{};
        };

        class colored_rectangle
        {
        public:
            bool operator () (viewport_size2d viewport) noexcept;

            void draw(pxrectangle rc, gl::rgba_colorf_t colorf) const noexcept
            {
                lib_.use();
                lib_.frag.u_color.store(colorf);
                lib_.vert.u_position.store(rc.position);
                lib_.vert.u_size.store(rc.sizes);
                private_detail_shader::draw(lib_);
            }

            template<class Fn>
            decltype(auto) apply(Fn fn) noexcept
            {
                return fn();
            }

        private:
            shader_library<vert::positioned_rectangle, frag::default_color> lib_{};
        };
    }
}