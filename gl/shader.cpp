#include "shader.h"

#include <core/small_vector.h>
#include <core/debug.h>


namespace gl
{
    namespace
    {
        [[nodiscard]]
        bool compile_status(shader_resource shader) noexcept
        {
            GLint status{ 0 };
            glGetShaderiv(to_underlying(shader), GL_COMPILE_STATUS, &status);
            return !!status;
        }

        [[nodiscard]]
        size_t compile_log_size(shader_resource shader) noexcept
        {
            GLint size{ 0 };
            glGetShaderiv(to_underlying(shader), GL_INFO_LOG_LENGTH, &size);
            return narrow_cast<size_t>(size);
        }

        size_t compile_log_read(shader_resource shader, std::span<GLchar> chars) noexcept
        {
            auto size = narrow_cast<GLsizei>(chars.size());
            glGetShaderInfoLog(to_underlying(shader), size, &size, chars.data());
            return narrow_cast<size_t>(size);
        }

        [[nodiscard]]
        bool link_status(shaders_program_resource program)  noexcept
        {
            GLint status{ 0 };
            glGetProgramiv(to_underlying(program), GL_LINK_STATUS, &status);
            return !!status;
        }

        using location_index_t = GLuint;
        static_assert(std::is_same_v<location_index_t, std::underlying_type_t<attribute_location>>);
        static_assert(std::is_same_v<location_index_t, std::underlying_type_t<uniform_location>>);

        using location_detail_getter_t = decltype(glGetActiveAttrib);
        static_assert(std::is_same_v<location_detail_getter_t, decltype(glGetActiveUniform)>);

        [[nodiscard]]
        bool test_location
        (
            location_detail_getter_t get_location_detail,
            shaders_program_resource program,
            location_index_t location,
            glsl_typeid test_typeid,
            zstring_view test_name
        ) noexcept
        {
            constexpr size_t name_buffer_static_size{ 4 * sizeof(size_t) };

            small_vector<GLchar, name_buffer_static_size> name_buffer{};
            name_buffer.reserve(test_name.size() + 2u);

            GLsizei name_size{ 0 };
            GLenum type_id{ 0 };
            GLint size{ 0 };

            get_location_detail
            (
                to_underlying(program),
                location,
                narrow_cast<GLsizei>(name_buffer.capacity()),
                &name_size,
                &size,
                &type_id,
                name_buffer.data()
            );

            const string_view name
            {
                std::as_const(name_buffer).data(),
                narrow_cast<size_t>(name_size)
            };

            const auto test0 = (size == 1u);
            const auto test1 = test0 && (type_id == to_underlying(test_typeid));
            const auto test2 = test1 && (name == test_name.as_string_view());

            return test2;
        }


        template<class LocationType>
        struct location_traits;

        template<>
        struct location_traits<attribute_location>
        {
            static constexpr auto getter = glGetAttribLocation;
            static constexpr auto detail_getter = glGetActiveAttrib;
        };

        template<>
        struct location_traits<uniform_location>
        {
            static constexpr auto getter = glGetUniformLocation;
            static constexpr auto detail_getter = glGetActiveUniform;
        };

        template <class LocationType>
        constexpr auto location_getter_v = location_traits<LocationType>::getter;

        template <class LocationType>
        constexpr auto location_detail_getter_v = location_traits<LocationType>::detail_getter;

        template <class LocationType>
        bool test_location
        (
            shaders_program_resource program,
            LocationType location,
            glsl_typeid test_typeid,
            zstring_view test_name
        )
        {
            return test_location
            (
                location_detail_getter_v<LocationType>,
                program,
                to_underlying(location),
                test_typeid,
                test_name
            );
        }

        template<class LocationType>
        [[nodiscard]] LocationType get_location(shaders_program_resource program, zstring_view name) noexcept
        {
            const auto location = location_getter_v<LocationType>(to_underlying(program), name.c_str());

            static_assert(std::is_signed_v<decltype(location)>);
            static_assert(std::is_unsigned_v<location_index_t>);
            static_assert(std::is_same_v<location_index_t, std::underlying_type_t<LocationType>>);
            return static_cast<LocationType>(narrow_cast<location_index_t>(location));
        }
    }

    void close(shader_resource shader) noexcept
    {
        if (has_value(shader))
        {
            glDeleteShader(to_underlying(shader));
        }
    }

    void set_source(shader_resource shader, string_view source) noexcept
    {
        const auto source_data = source.data();
        const auto source_length = narrow_cast<GLint>(source.size());
        glShaderSource(to_underlying(shader), 1, &source_data, &source_length);
    }

    bool compile(shader_resource shader) noexcept
    {
        glCompileShader(to_underlying(shader));
        return compile_status(shader);
    }

    shader_t create_shader(shader_type type) noexcept
    {
        return
        {
            resource_construct,
            underlying_cast<shader_resource>(glCreateShader(to_underlying(type)))
        };
    }

    void close(shaders_program_resource program) noexcept
    {
        if (has_value(program))
        {
            glDeleteProgram(to_underlying(program));
        }
    }

    void attach_shader(shaders_program_resource program, shader_resource shader) noexcept
    {
        glAttachShader(to_underlying(program), to_underlying(shader));
    }

    bool compile(shaders_program_resource program, string_view source, shader_type type) noexcept
    {
        if (const auto shader = create_shader(type))
        {
            set_source(shader, source);

            if (compile(shader))
            {
                attach_shader(program, shader);
                return true;
            }
            else
            {
                const auto log_size = compile_log_size(shader);

                if (log_size > 1u)
                {
                    std::basic_string<GLchar> chars(log_size, GLchar{});
                    compile_log_read(shader, chars);
                    output_debug_string("GLSL {}\n", chars.c_str());
                }
            }
        }

        return false;
    }

    bool link(shaders_program_resource program) noexcept
    {
        glLinkProgram(to_underlying(program));
        return link_status(program);
    }

    shaders_program_t create_shaders_program() noexcept
    {
        return
        {
            resource_construct,
            underlying_cast<shaders_program_resource>(glCreateProgram())
        };
    }

    shaders_program_t create_shaders_program(string_view vertex, string_view fragment) noexcept
    {
        auto program = create_shaders_program();
        D_ASSERT(program);

        if (program)
        {
            const auto vertext_ok = compile(program, vertex, shader_type::vertex);
            const auto fragment_ok = vertext_ok && compile(program, fragment, shader_type::fragment);
            const auto link_ok = fragment_ok && link(program);

            D_ASSERT(link_ok);

            if (!link_ok)
            {
                program.reset();
            }
        }

        return program;
    }

    attribute_location get_attribute_location(shaders_program_resource program, zstring_view name) noexcept
    {
        return get_location<attribute_location>(program, name);
    }

    bool test_attribute(shaders_program_resource program, attribute_location location, glsl_typeid test_typeid, zstring_view test_name) noexcept
    {
        return test_location(program, location, test_typeid, test_name);
    }

    uniform_location get_uniform_location(shaders_program_resource program, zstring_view name) noexcept
    {
        return get_location<uniform_location>(program, name);
    }

    bool test_uniform(shaders_program_resource program, uniform_location location, glsl_typeid test_typeid, zstring_view test_name) noexcept
    {
        return test_location(program, location, test_typeid, test_name);
    }
}


