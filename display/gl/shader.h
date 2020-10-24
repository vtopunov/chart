#pragma once

#include <core/underlying_cast.h>
#include <core/resouce.h>
#include <core/zstring_view.h>

#include <display/gl/type_id.h>

namespace display
{
    namespace gl
    {
        using zstring_view = basic_zstring_view<GLchar>;
        using string_view = std::basic_string_view<GLchar>;

        enum class shader_resource : GLuint
        {};

        void close(shader_resource shader) noexcept;

        void set_source(shader_resource shader, string_view source) noexcept;

        bool compile(shader_resource shader) noexcept;

        using shader_t = unique_resource<shader_resource>;

        enum class shader_type : GLenum
        {
            fragment = GL_FRAGMENT_SHADER,
            vertex = GL_VERTEX_SHADER
        };

        [[nodiscard]]
        shader_t create_shader(shader_type type) noexcept;

        enum class shaders_program_resource : GLuint
        {};

        void close(shaders_program_resource program) noexcept;

        void attach_shader(shaders_program_resource program, shader_resource shader) noexcept;

        bool compile(shaders_program_resource program, string_view source, shader_type type) noexcept;

        bool link(shaders_program_resource program) noexcept;

        inline void use(shaders_program_resource program) noexcept
        {
            glUseProgram(to_underlying(program));
        }

        using shaders_program_t = unique_resource<shaders_program_resource>;

        [[nodiscard]]
        shaders_program_t create_shaders_program() noexcept;

        [[nodiscard]]
        shaders_program_t create_shaders_program(string_view vertex, string_view fragment) noexcept;

        enum class attribute_location : GLuint
        {};

        [[nodiscard]]
        attribute_location get_attribute_location(shaders_program_resource program, zstring_view name) noexcept;

        [[nodiscard]]
        bool test_attribute
        (
            shaders_program_resource program,
            attribute_location location,
            type_id test_type_id,
            zstring_view test_name
         ) noexcept;

        template<type_id TypeId, class Location>
        struct shader_object
        {
            static constexpr auto type_index = TypeId;
            static constexpr auto element_type_index = tuple_element_type_id_v<type_index>;
            using element_type = type_t<element_type_index>;
            static constexpr auto element_type_size = sizeof(element_type);
            static constexpr auto tuple_size = tuple_size_v<type_index>;

            using location_t = Location;
            using location_num_t = std::underlying_type_t<location_t>;
            using location_int_t = std::make_signed_t<location_num_t>;
            using location_uint_t = std::make_unsigned_t<location_num_t>;

            location_t location;

            [[nodiscard]]
            constexpr location_num_t location_as_num() const noexcept
            {
                return to_underlying(location);
            }

            [[nodiscard]]
            constexpr location_int_t location_as_int() const noexcept
            {
                return narrow_cast<location_int_t>(location_as_num());
            }

            [[nodiscard]]
            constexpr location_uint_t location_as_uint() const noexcept
            {
                return narrow_cast<location_uint_t>(location_as_num());
            }
        };

        template<type_id id>
        using attribute = shader_object<id, attribute_location>;

        template<type_id id> [[nodiscard]]
        attribute<id> get_attribute(shaders_program_resource program, zstring_view name) noexcept
        {
             const attribute<id> result
             { 
                 .location{ get_attribute_location(program, name) }
             };

             D_ASSERT(test_attribute(program, result.location, id, name));
 
             return result;
        }

        using attribute_vec2f_t = attribute<type_id::vec2f>;
        using attribute_vec3f_t = attribute<type_id::vec3f>;
        using attribute_vec4f_t = attribute<type_id::vec4f>;

        template<type_id id> [[nodiscard]]
        void set_pointer(attribute<id> a, tuple_const_pointer_t<id> p, size_t stride = {}) noexcept
        {
            constexpr auto tuple_size = narrow_cast<GLint>(a.tuple_size);
            constexpr auto type_index = to_underlying(a.element_type_index);

            const auto stride_bytes = narrow_cast<GLsizei>(size_mul<a.element_type_size>(stride));

            glVertexAttribPointer
            (
                a.location_as_uint(),
                tuple_size,
                type_index,
                GL_FALSE, 
                stride_bytes,
                p
            );
        }

        template<type_id id>
        void enable_array(attribute<id> a) noexcept
        {
            glEnableVertexAttribArray(a.location_as_uint());
        }

        enum class uniform_location : GLuint
        {};

        [[nodiscard]]
        uniform_location get_uniform_location(shaders_program_resource program, zstring_view name) noexcept;

        [[nodiscard]]
        bool test_uniform
        (
            shaders_program_resource program,
            uniform_location location,
            type_id test_type_id,
            zstring_view test_name
        ) noexcept;

        template<type_id id>
        using uniform = shader_object<id, uniform_location>;

        template<type_id id> [[nodiscard]]
        uniform<id> get_uniform(shaders_program_resource program, zstring_view name) noexcept
        {
            const uniform<id> result
            {
                .location{ get_uniform_location(program, name) }
            };

            D_ASSERT(test_uniform(program, result.location, id, name));

            return result;
        }

        using uniform_vec4f_t = uniform<type_id::vec4f>;
        using const_span_vec4f_t = tuple_const_span_t<type_id::vec4f>;

        inline void set(uniform_vec4f_t u, const_span_vec4f_t span) noexcept
        {
            glUniform4fv(u.location_as_int(), 1, span.data());
        }

        using uniform_sampler2D_t = uniform<type_id::sampler2D>;
        using sampler2D_value_t = tuple_element_type_t<type_id::sampler2D>;

        inline void set(uniform_sampler2D_t u, sampler2D_value_t unit) noexcept
        {
            glUniform1i(u.location_as_int(), unit);
        }

        namespace literals
        {
            [[nodiscard]]
            constexpr string_view operator"" _glsl(const GLchar * source, size_t length) noexcept
            {
                return {source, length};
            }
        }
    }

    namespace gl_literals
    {
        using namespace gl::literals;
    }
}
