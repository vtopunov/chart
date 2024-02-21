#pragma once

#include <utility/shader_library.h>

#include <widget/fwd.h>


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


            template<class VS, class FS>
            [[nodiscard]] bool initialize_lib(shader_library<VS, FS>& lib, pxsize2d viewport_sizes) noexcept
            {
                if (lib || lib.build()) [[likely]]
                {
                    lib.use();
                    lib.vert.u_viewport.store(viewport_sizes);
                    return true;
                }

                return false;
            }

            template<class VS, class FS>
            struct widget_shader_user_base
            {
                using shader_library_type = shader_library<VS, FS>;
                using vertex_shader_type = typename shader_library_type::vertex_shader_type;
                using fragment_shader_type = typename shader_library_type::fragment_shader_type;

                template<class Derived>
                constexpr const Derived& as() const noexcept
                {
                    return identical_derived_cast<const Derived&>(*this);
                }

                void draw() const noexcept
                {
                    D_ASSERT(all_is_initialized(library));
                    attribute_frame::vertex_buffer_user::draw();
                }

                shader_library<VS, FS> library{};
            };

            template<class VS, class FS>
            struct default_widget_shader_user;

            template<class Derived, class VS, class FS>
            struct widget_shader_user : widget_shader_user_base<VS, FS>
            {
                using widget_shader_user_base<VS, FS>::library;

                const Derived& store(pxpoint2d position) const noexcept
                {
                    library.vert.u_position.store(position);
                    return self();
                }

                const Derived& store(pxsize2d sizes) const noexcept
                {
                    library.vert.u_size.store(sizes);
                    return self();
                }

                const Derived& store(pxrectangle rc) const noexcept
                {
                    return store(rc.position).store(rc.sizes);
                }

                const Derived& store(rgba_colorf_t colorf) const noexcept
                {
                    library.frag.u_color.store(colorf);
                    return self();
                }

                const Derived& store(gl::texture2d_resource texture) const noexcept
                {
                    library.frag.s_texture.store(texture);
                    return self();
                }

                const Derived& store(gl::texture2d_resources texture) const noexcept
                {
                    return store(sizes(texture)).store(static_cast<gl::texture2d_resource>(texture));
                }

                constexpr const Derived& self() const noexcept
                {
                    return identical_derived_cast<const Derived&>(*this);
                }

                void draw() const noexcept
                {
                    D_ASSERT(all_is_initialized(library));
                    attribute_frame::vertex_buffer_user::draw();
                }
            };

            template<class VS, class FS>
            struct default_widget_shader_user : widget_shader_user<default_widget_shader_user<VS, FS>, VS, FS>
            {};

            template<class VS, class FS>
            class widget_shader_library
            {
            public:
                using shader_user_type = default_widget_shader_user<VS, FS>;
                using vertex_shader_type = typename shader_user_type::vertex_shader_type;
                using fragment_shader_type = typename shader_user_type::fragment_shader_type;

                bool operator()(viewport_size2d viewport) noexcept
                {
                    return initialize_lib(u_.library, viewport);
                }

                const shader_user_type& use() const noexcept
                {
                    u_.library.use();
                    u_.library.vert.a_frame.bind();
                    return u_;
                }

                constexpr noapply_t apply(no_overload) const noexcept
                {
                    return noapply;
                }

            private:
                shader_user_type u_{};
            };

            template<class Derived, class Lib>
            using widget_shader_user_for_t = widget_shader_user<Derived, typename Lib::vertex_shader_type, typename Lib::fragment_shader_type>;
        }

        using private_detail_shader::widget_shader_library;
        using private_detail_shader::widget_shader_user;
        using private_detail_shader::default_widget_shader_user;
        using private_detail_shader::widget_shader_user_for_t;

        using luminance8_texture_mix_color = widget_shader_library<vert::positioned_texture, frag::luminance8_texture_mix_color>;
        using colored_rectangle = widget_shader_library<vert::positioned_rectangle, frag::default_color>;
    }

    using shader::widget_shader_library;
    using shader::widget_shader_user;
    using shader::default_widget_shader_user;
    using shader::widget_shader_user_for_t;
}

using widget::widget_shader_library;
using widget::widget_shader_user;
using widget::default_widget_shader_user;
using widget::widget_shader_user_for_t;