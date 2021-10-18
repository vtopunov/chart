#pragma once

#include <core/vec.h>
#include <display/gl/config.h>

namespace display
{
    namespace gl
    {
        using vec2i = vec2<GLint>;
        using vec2f = vec2<GLfloat>;
        using vec2b = vec2<GLboolean>;
      
        using vec2i_view = std::span<const GLint, 2u>;
        using vec2f_view = std::span<const GLfloat, 2u>;
        using vec2b_view = std::span<const GLboolean, 2u>;

        using vec3i_view = std::span<const GLint, 3u>;
        using vec3f_view = std::span<const GLfloat, 3u>;
        using vec3b_view = std::span<const GLboolean, 3u>;

        using vec4i_view = std::span<const GLint, 4u>;
        using vec4f_view = std::span<const GLfloat, 4u>;
        using vec4b_view = std::span<const GLboolean, 4u>;

        enum class glsl_typeid : GLenum
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

        template<glsl_typeid id>
        struct select_type_for_glsl
        {};

        template<>
        struct select_type_for_glsl<glsl_typeid::byte>
        {
            using type = GLbyte;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::ubyte>
        {
            using type = GLubyte;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::sshort>
        {
            using type = GLshort;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::ushort>
        {
            using type = GLushort;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::sint>
        {
            using type = GLint;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::uint>
        {
            using type = GLuint;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::real>
        {
            using type = GLfloat;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::boolean>
        {
            using type = GLboolean;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::vec2f>
        {
            using type = vec2f;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::vec2i>
        {
            using type = vec2i;
        };

        template<>
        struct select_type_for_glsl<glsl_typeid::vec2b>
        {
            using type = vec2b;
        };

        template<glsl_typeid id>
        using glsl_type_t = typename select_type_for_glsl<id>::type;

        template <glsl_typeid Value>
        using glsl_typeid_constant = std::integral_constant<glsl_typeid, Value>;

        template<class T>
        struct select_glsl_typeid
        {};

        template<>
        struct select_glsl_typeid<GLbyte> : glsl_typeid_constant<glsl_typeid::byte>
        {};

        template<>
        struct select_glsl_typeid<GLubyte> : glsl_typeid_constant<glsl_typeid::ubyte>
        {};

        template<>
        struct select_glsl_typeid<GLshort> : glsl_typeid_constant<glsl_typeid::sshort>
        {};

        template<>
        struct select_glsl_typeid<GLushort> : glsl_typeid_constant<glsl_typeid::ushort>
        {};
        
        template<>
        struct select_glsl_typeid<GLint> : glsl_typeid_constant<glsl_typeid::sint>
        {};
        
        template<>
        struct select_glsl_typeid<GLuint> : glsl_typeid_constant<glsl_typeid::uint>
        {};

        template<>
        struct select_glsl_typeid<GLfloat> : glsl_typeid_constant<glsl_typeid::real>
        {};

        template<>
        struct select_glsl_typeid<vec2f> : glsl_typeid_constant<glsl_typeid::vec2f>
        {};
        
        template<>
        struct select_glsl_typeid<vec2i> : glsl_typeid_constant<glsl_typeid::vec2i>
        {};

        template<>
        struct select_glsl_typeid<vec2b> : glsl_typeid_constant<glsl_typeid::vec2b>
        {};

        template<class T>
        constexpr auto glsl_typeid_v = select_glsl_typeid<T>::value;

        [[nodiscard]]
        constexpr size_t glsl_tuple_size(glsl_typeid id) noexcept
        {
            switch (id)
            {
                case glsl_typeid::vec4f:
                case glsl_typeid::vec4i:
                case glsl_typeid::vec4b:
                case glsl_typeid::mat2f:
                    return 4u;

                case glsl_typeid::vec3f:
                case glsl_typeid::vec3i:
                case glsl_typeid::vec3b:
                    return 3u;

                case glsl_typeid::vec2f:
                case glsl_typeid::vec2i:
                case glsl_typeid::vec2b:
                    return 2u;

                case glsl_typeid::mat4f:
                    return 4u * 4u;

                case glsl_typeid::mat3f:
                    return 3u * 3u;

            }

            return 1u;
        };

        template<glsl_typeid id>
        inline constexpr auto glsl_tuple_size_v = glsl_tuple_size(id);

        [[nodiscard]]
        constexpr glsl_typeid glsl_tuple_element_typeid(glsl_typeid id) noexcept
        {
            switch (id)
            {
                case glsl_typeid::vec2f:
                case glsl_typeid::vec3f:
                case glsl_typeid::vec4f:
                case glsl_typeid::mat2f:
                case glsl_typeid::mat3f:
                case glsl_typeid::mat4f:
                    return glsl_typeid::real;
                case glsl_typeid::vec2i:
                case glsl_typeid::vec3i:
                case glsl_typeid::vec4i:
                case glsl_typeid::sampler2D:
                case glsl_typeid::samplerCube:
                    return glsl_typeid::sint;
                case glsl_typeid::vec2b:
                case glsl_typeid::vec3b:
                case glsl_typeid::vec4b:
                    return glsl_typeid::boolean;
            }

            return id;
        }

        template<glsl_typeid id>
        inline constexpr auto glsl_tuple_element_typeid_v = glsl_tuple_element_typeid(id);

        template<glsl_typeid id>
        using glsl_tuple_element_type_t = glsl_type_t<glsl_tuple_element_typeid_v<id>>;

        [[nodiscard]]
        constexpr bool is_sampler(glsl_typeid id) noexcept
        {
            return glsl_typeid::sampler2D == id || glsl_typeid::samplerCube == id;
        }
    }
}