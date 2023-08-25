#pragma once

#include <core/vec2.h>

#include <gl/config.h>


namespace gl
{
    using vec2i = vec2<GLint>;
    using vec2f = vec2<GLfloat>;
    using vec2b = vec2<GLboolean>;

    using const_span2i = span<const GLint, 2_uz>;
    using const_span2f = span<const GLfloat, 2_uz>;
    using const_span2b = span<const GLboolean, 2_uz>;

    using const_span3i = span<const GLint, 3_uz>;
    using const_span3f = span<const GLfloat, 3_uz>;
    using const_span3b = span<const GLboolean, 3_uz>;

    using const_span4i = span<const GLint, 4_uz>;
    using const_span4f = span<const GLfloat, 4_uz>;
    using const_span4b = span<const GLboolean, 4_uz>;

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


    namespace private_detail_glsl_type_by_id
    {
        template<glsl_typeid id>
        struct select_type_for_glsl
        {
            using type = void;
        };

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

        template<>
        struct select_type_for_glsl<glsl_typeid::sampler2D> : select_type_for_glsl<glsl_typeid::sint>
        {};

        template<>
        struct select_type_for_glsl<glsl_typeid::samplerCube> : select_type_for_glsl<glsl_typeid::sint>
        {};
    }

    template<glsl_typeid id>
    using glsl_type_t = typename private_detail_glsl_type_by_id::select_type_for_glsl<id>::type;


    template <glsl_typeid Value>
    using glsl_typeid_constant = std::integral_constant<glsl_typeid, Value>;

    namespace private_detail_glsl_typeid_by_type
    {
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
    }

    template<class T>
    constexpr auto glsl_typeid_v = private_detail_glsl_typeid_by_type::select_glsl_typeid<T>::value;


    [[nodiscard]]
    constexpr size_t glsl_tuple_size(glsl_typeid id) noexcept
    {
        switch (id)
        {
            case glsl_typeid::vec2f:
            case glsl_typeid::vec2i:
            case glsl_typeid::vec2b:
                return 2_uz;

            case glsl_typeid::vec3f:
            case glsl_typeid::vec3i:
            case glsl_typeid::vec3b:
                return 3_uz;

            case glsl_typeid::vec4f:
            case glsl_typeid::vec4i:
            case glsl_typeid::vec4b:
            case glsl_typeid::mat2f:
                return 4_uz;

            case glsl_typeid::mat3f:
                return 3_uz * 3_uz;

            case glsl_typeid::mat4f:
                return 4_uz * 4_uz;

            default:
                break;
        }

        return 1_uz;
    };

    template<glsl_typeid id>
    constexpr auto glsl_tuple_size_v = glsl_tuple_size(id);

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

            default:
                break;
        }

        return id;
    }

    template<glsl_typeid id>
    constexpr auto glsl_tuple_element_typeid_v = glsl_tuple_element_typeid(id);

    template<glsl_typeid id>
    using glsl_tuple_element_type_t = glsl_type_t<glsl_tuple_element_typeid_v<id>>;


    namespace private_detail_glsl_view
    {
        template<glsl_typeid id, size_t tuple_size>
        struct select_glsl_view3
        {
            using type = span<std::add_const_t<glsl_tuple_element_type_t<id>>, tuple_size>;
        };

        template<glsl_typeid id, size_t tuple_size, class T, class ViewT>
        struct select_glsl_view2
        {
            using type = ViewT;
        };

        template<glsl_typeid id, size_t tuple_size, class T>
        struct select_glsl_view2<id, tuple_size, T, T> : select_glsl_view3<id, tuple_size>
        {};

        template<glsl_typeid id, size_t tuple_size, class T>
        struct select_glsl_view1 : select_glsl_view2<id, tuple_size, T, std::remove_cvref_t<typename T::view_type>>
        {};

        template<glsl_typeid id, size_t tuple_size>
        struct select_glsl_view1<id, tuple_size, void> : select_glsl_view3<id, tuple_size>
        {};

        template<glsl_typeid id, size_t tuple_size>
        struct select_glsl_view0 : select_glsl_view1<id, tuple_size, glsl_type_t<id>>
        {};

        template<glsl_typeid id>
        struct select_glsl_view0<id, 1_uz>
        {
            using type = glsl_type_t<id>;
        };
    }

    template<glsl_typeid id>
    using glsl_view_t = typename private_detail_glsl_view::select_glsl_view0<id, glsl_tuple_size_v<id>>::type;
}
