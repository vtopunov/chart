#include "shader.h"

#include <string>

#include <core/small_vector.h>

#include <debug/debug.h>


namespace gl
{
    namespace
    {
        namespace resource
        {
            [[nodiscard]]
            GLint ivalue(shader_resource r, GLenum e, GLint defValue = {}) noexcept
            {
                glGetShaderiv(to_underlying(r), e, std::addressof(defValue));
                return defValue;
            }

            [[nodiscard]]
            GLint ivalue(program_resource r, GLenum e, GLint defValue = {}) noexcept
            {
                glGetProgramiv(to_underlying(r), e, std::addressof(defValue));
                return defValue;
            }

            [[nodiscard]]
            GLsizei log(shader_resource r, GLsizei size, char* s) noexcept
            {
                glGetShaderInfoLog(to_underlying(r), size, &size, s);
                return size;
            }

            [[nodiscard]]
            GLsizei log(program_resource r, GLsizei size, char* s) noexcept
            {
                glGetProgramInfoLog(to_underlying(r), size, &size, s);
                return size;
            }
        }

        template<class R>
        [[nodiscard]] size_t size_log(const R r) noexcept
        {
            return narrow<size_t>(resource::ivalue(r, GL_INFO_LOG_LENGTH));
        }

        template<class R>
        size_t log(const R r, span<char> chars) noexcept
        {
            return narrow<size_t>(resource::log(r, narrow<GLsizei>(chars.size()), chars.data()));
        }

        template<class R>
        [[nodiscard]] std::string log(const R r) noexcept
        {
            std::string log_string(size_log(r), '\0');
            log_string.erase(log(r, log_string));
            return log_string;
        }

        [[nodiscard]]
        bool compile_status(shader_resource shader) noexcept
        {
            return !!resource::ivalue(shader, GL_COMPILE_STATUS);
        }

        [[nodiscard]]
        bool link_status(program_resource program)  noexcept
        {
            return !!resource::ivalue(program, GL_LINK_STATUS);
        }

        using location_detail_getter_t = decltype(glGetActiveAttrib);
        static_assert(std::is_same_v<location_detail_getter_t, decltype(glGetActiveUniform)>);

        [[nodiscard]]
        bool test_location
        (
            location_detail_getter_t get_location_detail,
            program_resource program,
            location_index_t location,
            glsl_typeid test_typeid,
            std::string_view test_name
        ) noexcept
        {
            constexpr size_t name_buffer_static_size{ 4 * sizeof(size_t) };

            small_vector<char, name_buffer_static_size> name_buffer{};
            name_buffer.reserve(test_name.size() + 2_uz);

            GLsizei name_size{ 0 };
            GLenum type_id{ 0 };
            GLint size{ 0 };

            get_location_detail
            (
                to_underlying(program),
                location,
                narrow<GLsizei>(name_buffer.capacity()),
                &name_size,
                &size,
                &type_id,
                name_buffer.data()
            );

            const std::string_view name
            {
                std::as_const(name_buffer).data(),
                narrow<size_t>(name_size)
            };

            const auto test0 = (size == 1_uz);
            const auto test1 = test0 && (type_id == to_underlying(test_typeid));
            const auto test2 = test1 && (name == test_name);

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
        [[nodiscard]] bool test_location
        (
            program_resource program,
            LocationType location,
            glsl_typeid test_typeid,
            std::string_view test_name
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
        [[nodiscard]] LocationType get_location(program_resource program, zstring_view name) noexcept
        {
            const auto location = location_getter_v<LocationType>(to_underlying(program), name.c_str());
            static_assert(is_same_uncv_v<std::make_unsigned_t<decltype(location)>, std::underlying_type_t<LocationType>>);
            return narrow<LocationType>(location);
        }
    }

    void set_source(shader_resource shader, source_view source) noexcept
    {
        const auto source_data = source.data();
        const auto source_length = narrow<GLint>(source.size());
        glShaderSource(to_underlying(shader), 1, &source_data, &source_length);
    }

    bool compile(shader_resource shader) noexcept
    {
        glCompileShader(to_underlying(shader));
        return compile_status(shader);
    }

    void shader_resource_deleter::operator()(shader_resource shader) const noexcept
    {
        glDeleteShader(to_underlying(shader));
    }

    shader create_shader(shader_type type) noexcept
    {
        return shader{ underlying_cast<shader_resource>(glCreateShader(to_underlying(type))) };
    }

    void attach_shader(program_resource program, shader_resource shader) noexcept
    {
        glAttachShader(to_underlying(program), to_underlying(shader));
    }

    bool compile(program_resource program, source_view source, shader_type type) noexcept
    {
        if (const auto shader = create_shader(type)) [[likely]]
        {
            set_source(shader, source);

            if (compile(shader)) [[likely]]
            {
                attach_shader(program, shader);
                return true;
            }
            else
            {
                e_debug("GLSL: {}", log(view(shader)));
            }
        }

        return false;
    }

    bool link(program_resource program) noexcept
    {
        glLinkProgram(to_underlying(program));
        return link_status(program);
    }

    program_resource current_program() noexcept
    {
        using programi_t = std::make_signed_t<std::underlying_type_t<program_resource>>;
        constexpr auto null_programi = static_cast<programi_t>(program_resource::null);
        programi_t programi{ null_programi };
        glGetIntegerv(GL_CURRENT_PROGRAM, std::addressof(programi));
        return static_cast<program_resource>(programi);
    }

    void program_resource_deleter::operator()(program_resource program) const noexcept
    {
        glDeleteProgram(to_underlying(program));
    }

    program create_program() noexcept
    {
        return program{ underlying_cast<program_resource>(glCreateProgram()) };
    }

    program create_program(vertex_source_view vertex, fragment_source_view fragment) noexcept
    {
        auto program = create_program();

        if (program) [[likely]]
        {
            const auto ok
                = compile(program, vertex)
                && compile(program, fragment)
                && link(program);

            if (!ok) [[unlikely]]
            {
                e_debug("GL program: {}", log(view(program)));
                program.reset();
            }
        }

        return program;
    }

    attribute_location get_attribute_location(program_resource program, zstring_view name) noexcept
    {
        return get_location<attribute_location>(program, name);
    }

    uniform_location get_uniform_location(program_resource program, zstring_view name) noexcept
    {
        return get_location<uniform_location>(program, name);
    }

    bool test_uniform(program_resource program, uniform_location location, glsl_typeid test_typeid, std::string_view test_name) noexcept
    {
        return test_location(program, location, test_typeid, test_name);
    }
}


