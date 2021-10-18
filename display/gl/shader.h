#pragma once

#include <core/underlying_cast.h>
#include <core/resouce.h>
#include <core/zstring_view.h>

#include <display/gl/glsl_typeid.h>

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
            glsl_typeid test_typeid,
            zstring_view test_name
         ) noexcept;
     
        enum class uniform_location : GLuint
        {};

        [[nodiscard]]
        uniform_location get_uniform_location(shaders_program_resource program, zstring_view name) noexcept;

        [[nodiscard]]
        bool test_uniform
        (
            shaders_program_resource program,
            uniform_location location,
            glsl_typeid test_typeid,
            zstring_view test_name
        ) noexcept;

        template<glsl_typeid id>
        struct uniform
        {
            using location_t = uniform_location;
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

            bool test(shaders_program_resource program, zstring_view name) const noexcept
            {
                return test_uniform(program, location, id, name);
            }
        };

        template<glsl_typeid id> [[nodiscard]]
        uniform<id> get_uniform(shaders_program_resource program, zstring_view name) noexcept
        {
            const uniform<id> result
            {
                .location{ get_uniform_location(program, name) }
            };

            D_ASSERT(result.test(program, name));

            return result;
        }

        using uniform_vec4f_t = uniform<glsl_typeid::vec4f>;

        inline void set(uniform_vec4f_t u, vec4f_view view) noexcept
        {
            glUniform4fv(u.location_as_int(), 1, std::data(view));
        }

        using uniform_sampler2D_t = uniform<glsl_typeid::sampler2D>;
        using sampler2D_value_t = glsl_tuple_element_type_t<glsl_typeid::sampler2D>;

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

            [[nodiscard]]
            constexpr zstring_view operator"" _zsv(const GLchar * source, size_t length) noexcept
            {
                return { null_terminated_construct, source, length};
            }
        }
    }

    namespace gl_literals
    {
        using namespace gl::literals;
    }
}
