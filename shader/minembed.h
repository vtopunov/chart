#pragma once

#include <shader/common.h>


namespace shader_embed
{
    namespace vert
    {
        struct positioned_rectangle
        {
            static constexpr auto source = R"(
                precision mediump float;

                uniform vec2 u_position;
                uniform vec2 u_size;
                uniform vec2 u_viewport;

                attribute vec2 a_frame;

                void main()
                {
                    vec2 px_position = (2.0 * (a_frame * u_size + u_position)) / u_viewport;
                    gl_Position = vec4(px_position.x - 1.0, 1.0 - px_position.y, 0.0, 1.0);
                }
            )"_vert_glsl;

            struct exports
            {
                static constexpr shader_export::pxfvec position{ "u_position"_zsv };
                static constexpr shader_export::pxfvec sizes{ "u_size"_zsv };
                static constexpr shader_export::pxfvec viewport{ "u_viewport"_zsv };
                static constexpr shader_export::frame frame{ "a_frame"_zsv };

                template<class Fn>
                static constexpr decltype(auto) apply(Fn&& fn) noexcept
                {
                    return std::forward<Fn>(fn)(
                        position,
                        sizes,
                        viewport,
                        frame
                    );
                }

                template<class Lib>
                struct interface
                {
                    static constexpr shader_common::import_engine<Lib> import{};

                    template<class... Types>
                    auto position(const Types&... values) const noexcept -> decltype(exports::position.store(gl::uniform_location::invalid, values...))
                    {
                        constexpr auto store = import(exports::position);
                        return store(*this, values...);
                    }

                    template<class... Types>
                    auto sizes(const Types&... values) const noexcept -> decltype(exports::sizes.store(gl::uniform_location::invalid, values...))
                    {
                        constexpr auto store = import(exports::sizes);
                        return store(*this, values...);
                    }

                    template<class T>
                    auto geometry(const rectangle<T>& rc) const noexcept -> decltype((position(rc.position), sizes(rc.sizes)))
                    {
                        return (position(rc.position), sizes(rc.sizes));
                    }

                    template<class... Types>
                    auto viewport(const Types&... values) const noexcept -> decltype(exports::viewport.store(gl::uniform_location::invalid, values...))
                    {
                        constexpr auto store = import(exports::viewport);
                        return store(*this, values...);
                    }

                    void draw() const noexcept
                    {
                        D_ASSERT(shader_common::interface_to_library<Lib>(*this).debug_all_in_use());
                        exports::frame.draw();
                    }
                };
            };
        };

        struct positioned_texture
        {
            static constexpr auto source = R"(
                precision mediump float;

                uniform vec2 u_position;
                uniform vec2 u_size;
                uniform vec2 u_viewport;

                attribute vec2 a_frame;

                varying vec2 v_texture;

                void main()
                {
                    vec2 px_position = (2.0 * (a_frame * u_size + u_position)) / u_viewport;
                    gl_Position = vec4(px_position.x - 1.0, 1.0 - px_position.y, 0.0, 1.0);
                    v_texture = a_frame;
                }
            )"_vert_glsl;

            using exports = positioned_rectangle::exports;
        };
    }

    namespace frag
    {
        struct default_color
        {
            static constexpr auto source = R"(
                precision mediump float;

                uniform vec4 u_color;

                void main()
                {
                    gl_FragColor = u_color;
                }
            )"_frag_glsl;

            struct exports
            {
                static constexpr shader_export::color color{ "u_color"_zsv };

                template<class Fn>
                static constexpr decltype(auto) apply(Fn&& fn) noexcept
                {
                    return std::forward<Fn>(fn)(color);
                }

                template<class Lib>
                struct interface
                {
                    static constexpr shader_common::import_engine<Lib> import{};

                    template<class... Types>
                    auto color(const Types&... values) const noexcept -> decltype(exports::color.store(gl::uniform_location::invalid, values...))
                    {
                        constexpr auto store = import(exports::color);
                        return store(*this, values...);
                    }
                };
            };
        };

        struct default_texture
        {
            static constexpr auto source = R"(
                precision mediump float;

                uniform sampler2D s_texture;
                varying vec2 v_texture;

                void main()
                {
                    gl_FragColor = texture2D(s_texture, v_texture);
                }
            )"_frag_glsl;

            struct exports
            {
                static constexpr shader_export::sampler texture{ "s_texture"_zsv };

                template<class Fn>
                static constexpr decltype(auto) apply(Fn&& fn) noexcept
                {
                    return std::forward<Fn>(fn)(texture);
                }

                template<class Lib>
                struct interface
                {
                    template<class Texture>
                    auto texture(const Texture& tex) const noexcept -> decltype(gl::store_texture(GLenum{}, tex))
                    {
                        shader_common::store_sizes_if_support(shader_common::interface_to_library<Lib>(*this), tex);

                        constexpr auto tex_number = shader_common::texture_number<Lib>(exports::texture);
                        gl::store_texture(tex_number, tex);
                    }
                };
            };
        };

        struct inverted_texture
        {
            static constexpr auto source = R"(
                precision mediump float;
                
                uniform sampler2D s_texture;
                varying vec2 v_texture;
                
                void main()
                {
                    vec4 tex =  texture2D(s_texture, v_texture);
                    gl_FragColor = vec4(1.0 - tex.rgb, tex.a);
                }
            )"_frag_glsl;

            using exports = default_texture::exports;
        };

        struct luminance_texture
        {
            static constexpr auto source = R"(
                precision mediump float;

                uniform vec4 u_color;
                uniform sampler2D s_texture;

                varying vec2 v_texture;

                void main()
                {
                    vec4 tex = texture2D(s_texture, v_texture);
                    gl_FragColor = vec4(u_color.rgb, u_color.a * tex.r);
                }
            )"_frag_glsl;

            using exports = types_cat_t<default_color::exports, default_texture::exports>;
        };
    }
}


