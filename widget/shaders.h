#pragma once

#include <utility/shaders_library.h>


namespace widget
{
    namespace private_detail_shaders_library
    {
        template<class T>
        void initialize_ro_uniform(const px::uniform_vec2& vec, ::vec2<T> p) noexcept
        {
            D_ASSERT(!vec.uniform.__debug_is_stored());
            vec.store(std::move(p));
            D_ONLY_DEBUG(vec.uniform.__debug_set_ro());
        }

        inline void initialize_viewport(const vert::positioned_frame& vert, pxsize2d sizes) noexcept
        {
            initialize_ro_uniform(vert.u_viewport, sizes);
        }

        template<class VS, class FS>
        void initialize_lib_uniforms(const shaders_library<VS, FS>& lib, pxsize2d viewport_sizes) noexcept
        {
            lib.use();
            initialize_viewport(lib.vert, viewport_sizes);
        }

        template<class VS, class FS>
        bool initialize_lib(shaders_library<VS, FS>& lib, pxsize2d viewport_sizes) noexcept
        {            
            if (lib.build()) [[likely]]
            {
                initialize_lib_uniforms(lib, viewport_sizes);
                return true;
            }

            return false;
        }

#if D_IS_DEBUG
        template<gl::glsl_typeid type_id>
        [[nodiscard]] constexpr bool is_initialized(const gl::uniform<type_id>& u) noexcept
        {
            return u.__debug_is_stored();
        }

        [[nodiscard]] constexpr bool is_initialized(const px::uniform_vec2& vec) noexcept
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
        [[nodiscard]] constexpr bool all_is_initialized(const shaders_library<VS, FS>& lib) noexcept
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

        inline void draw(const vert::positioned_frame & vert) noexcept
        {
            vert.a_frame.draw();
        }

        template<class VS, class FS>
        void draw(const shaders_library<VS, FS>&lib) noexcept
        {
            D_ASSERT(all_is_initialized(lib));
            draw(lib.vert);
        }
    }

    class gray_texture_mix_color_lib
    {
    public:
        bool initialize(pxsize2d viewport_sizes) noexcept
        {
            return private_detail_shaders_library::initialize_lib(lib, viewport_sizes);
        }

        void draw(pxpoint2d position, gl::texture2d_resources texture, gl::rgba_colorf_view colorf) const noexcept
        {
            lib.use();
            lib.frag.u_color.store(colorf);
            lib.frag.s_texture.store(texture);
            lib.vert.u_position.store(position);
            lib.vert.u_size.store(sizes(texture));
            private_detail_shaders_library::draw(lib);
        }

    private:
        shaders_library<vert::positioned_texture, frag::gray_texture_mix_color> lib{};
    };

    class colored_rectangle_lib
    {
    public:
        bool initialize(pxsize2d viewport_sizes) noexcept
        {
            return private_detail_shaders_library::initialize_lib(lib, viewport_sizes);;
        }

        void draw(pxrectangle rc, gl::rgba_colorf_t colorf) const noexcept
        {
            lib.use();
            lib.frag.u_color.store(colorf);
            lib.vert.u_position.store(rc.position);
            lib.vert.u_size.store(rc.sizes);
            private_detail_shaders_library::draw(lib);
        }

    private:
        shaders_library<vert::positioned_rectangle, frag::default_color> lib{};
    };

    struct shaders
    {
        gray_texture_mix_color_lib gray_texture_mix_color;
        colored_rectangle_lib colored_rectangle;
    };
}