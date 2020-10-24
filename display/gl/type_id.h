#pragma once

#include <type_traits>
#include <span>

#include <display/gl/config.h>

namespace display
{
    namespace gl
    {
        enum class type_id : GLenum
        {
            boolean = GL_BOOL,
            byte = GL_BYTE,
            ubyte = GL_UNSIGNED_BYTE,
            sshort = GL_SHORT,
            ushort = GL_UNSIGNED_SHORT,
            sint = GL_INT,
            uint = GL_UNSIGNED_INT,
            real = GL_FLOAT,
            vec2f = GL_FLOAT_VEC2,
            vec3f = GL_FLOAT_VEC3,
            vec4f = GL_FLOAT_VEC4,
            vec2i = GL_INT_VEC2,
            vec3i = GL_INT_VEC3,
            vec4i = GL_INT_VEC4,
            vec2b = GL_BOOL_VEC2,
            vec3b = GL_BOOL_VEC3,
            vec4b = GL_BOOL_VEC4,
            mat2f = GL_FLOAT_MAT2,
            mat3f = GL_FLOAT_MAT3,
            mat4f = GL_FLOAT_MAT4,
            sampler2D = GL_SAMPLER_2D,
            samplerCube = GL_SAMPLER_CUBE
        };

        template<type_id id>
        struct type_select_type
        {};

        template<>
        struct type_select_type<type_id::byte>
        {
            using type = GLbyte;
        };

        template<>
        struct type_select_type<type_id::ubyte>
        {
            using type = GLubyte;
        };

        template<>
        struct type_select_type<type_id::sshort>
        {
            using type = GLshort;
        };

        template<>
        struct type_select_type<type_id::ushort>
        {
            using type = GLushort;
        };

        template<>
        struct type_select_type<type_id::sint>
        {
            using type = GLint;
        };

        template<>
        struct type_select_type<type_id::uint>
        {
            using type = GLuint;
        };

        template<>
        struct type_select_type<type_id::real>
        {
            using type = GLfloat;
        };

        template<>
        struct type_select_type<type_id::boolean>
        {
            using type = GLboolean;
        };

        consteval type_id select_type_id(GLbyte) noexcept
        {
            return type_id::byte;
        }

        consteval type_id select_type_id(GLubyte) noexcept
        {
            return type_id::ubyte;
        }

        consteval type_id select_type_id(GLshort) noexcept
        {
            return type_id::sshort;
        }

        consteval type_id select_type_id(GLushort) noexcept
        {
            return type_id::ushort;
        }

        consteval type_id select_type_id(GLint) noexcept
        {
            return type_id::sint;
        }

        consteval type_id select_type_id(GLuint) noexcept
        {
            return type_id::uint;
        }

        consteval type_id select_type_id(GLfloat) noexcept
        {
            return type_id::real;
        }

        [[nodiscard]]
        constexpr size_t tuple_size(type_id id) noexcept
        {
            switch (id)
            {
                case type_id::vec4f:
                case type_id::vec4i:
                case type_id::vec4b:
                case type_id::mat2f:
                    return 4u;

                case type_id::vec3f:
                case type_id::vec3i:
                case type_id::vec3b:
                    return 3u;

                case type_id::vec2f:
                case type_id::vec2i:
                case type_id::vec2b:
                    return 2u;

                case type_id::mat4f:
                    return 4u * 4u;

                case type_id::mat3f:
                    return 3u * 3u;

            }

            return 1u;
        };

        template<type_id id>
        inline constexpr auto tuple_size_v = tuple_size(id);

        [[nodiscard]]
        constexpr type_id tuple_element_type_id(type_id id) noexcept
        {
            switch (id)
            {
                case type_id::vec2f:
                case type_id::vec3f:
                case type_id::vec4f:
                case type_id::mat2f:
                case type_id::mat3f:
                case type_id::mat4f:
                    return type_id::real;
                case type_id::vec2i:
                case type_id::vec3i:
                case type_id::vec4i:
                case type_id::sampler2D:
                case type_id::samplerCube:
                    return type_id::sint;
                case type_id::vec2b:
                case type_id::vec3b:
                case type_id::vec4b:
                    return type_id::boolean;
            }

            return id;
        }

        template<type_id id>
        inline constexpr auto tuple_element_type_id_v = tuple_element_type_id(id);

        template<type_id id>
        using type_t = typename type_select_type<id>::type;

        template<type_id id>
        using tuple_element_type_t = type_t<tuple_element_type_id_v<id>>;

        template<type_id id>
        using tuple_span_t = std::span<tuple_element_type_t<id>, tuple_size_v<id>>;

        template<type_id id>
        using tuple_const_span_t = std::span<std::add_const_t<tuple_element_type_t<id>>, tuple_size_v<id>>;

        template<type_id id>
        using tuple_pointer_t = tuple_element_type_t<id>*;

        template<type_id id>
        using tuple_const_pointer_t = const tuple_element_type_t<id>*;

        [[nodiscard]]
        constexpr bool is_sampler(type_id id) noexcept
        {
            return type_id::sampler2D == id || type_id::samplerCube == id;
        }
    }
}