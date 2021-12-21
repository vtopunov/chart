#pragma once

#include <core/resouce.h>
#include <core/zstring_view.h>

#include <gl/glsl_typeid.h>

namespace gl
{
    using zstring_view = basic_zstring_view<GLchar>;
    using string_view = std::basic_string_view<GLchar>;

    enum class shader_resource : GLuint
    {
        null
    };

    void set_source(shader_resource shader, string_view source) noexcept;

    bool compile(shader_resource shader) noexcept;

    struct shader_resource_deleter
    {
        void operator () (shader_resource shader) const noexcept;
    };

    using shader_t = unique_resource<shader_resource, shader_resource_deleter>;

    enum class shader_type : GLenum
    {
        fragment = GL_FRAGMENT_SHADER,
        vertex = GL_VERTEX_SHADER
    };

    [[nodiscard]]
    shader_t create_shader(shader_type type) noexcept;

    enum class shaders_program_resource : GLuint
    {
        null
    };

    void attach_shader(shaders_program_resource program, shader_resource shader) noexcept;

    bool compile(shaders_program_resource program, string_view source, shader_type type) noexcept;

    bool link(shaders_program_resource program) noexcept;

    inline void use(shaders_program_resource program) noexcept
    {
        glUseProgram(to_underlying(program));
    }

    struct shaders_program_resource_deleter
    {
        void operator () (shaders_program_resource program) const noexcept;
    };

    using shaders_program_t = unique_resource<shaders_program_resource, shaders_program_resource_deleter>;

    [[nodiscard]]
    shaders_program_t create_shaders_program() noexcept;

    [[nodiscard]]
    shaders_program_t create_shaders_program(string_view vertex, string_view fragment) noexcept;


    using location_numer_t = GLuint;

    constexpr auto invalid_location_number = numeric_max_v<location_numer_t>;

    enum class attribute_location : location_numer_t
    {
        invalid = invalid_location_number
    };

    using invalidattribute_t = null_t<attribute_location>;   
    inline constexpr invalidattribute_t invalidattribute{};
    static_assert(attribute_location::invalid == invalidattribute);

    [[nodiscard]]
    attribute_location get_attribute_location(shaders_program_resource program, zstring_view name) noexcept;

    template<class... Names> [[nodiscard]]
    std::array<attribute_location, sizeof...(Names)>  get_attribute_locations(shaders_program_resource program, const Names&... names) noexcept
    {
         return { get_attribute_location(program, names)... };
    }

    [[nodiscard]]
    bool test_attribute
    (
        shaders_program_resource program,
        attribute_location location,
        glsl_typeid test_typeid,
        zstring_view test_name
    ) noexcept;


    enum class uniform_location : location_numer_t
    {
        invalid = invalid_location_number
    };

    using location_int_t = std::make_signed_t<location_numer_t>;
    static_assert(std::is_same_v<location_int_t, GLint>);

    [[nodiscard]]
    constexpr location_int_t location_as_int(uniform_location location) noexcept
    {
        return narrow_cast<location_int_t>(location);
    }

    [[nodiscard]]
    uniform_location get_uniform_location(shaders_program_resource program, zstring_view name) noexcept;

    inline void store_uniform_value(uniform_location u, GLint value) noexcept
    {
        glUniform1i(location_as_int(u), value);
    }

    inline void store_uniform_value(uniform_location u, GLfloat value) noexcept
    {
        glUniform1f(location_as_int(u), value);
    }

    inline void store_uniform_value(uniform_location u, const_span2i_t value) noexcept
    {
        glUniform2iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span2f_t value) noexcept
    {
        glUniform2fv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span3i_t value) noexcept
    {
        glUniform3iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span3f_t value) noexcept
    {
        glUniform3fv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span4i_t value) noexcept
    {
        glUniform4iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span4f_t value) noexcept
    {
        glUniform4fv(location_as_int(u), 1, std::data(value));
    }

    template<glsl_typeid id>
    struct store_uniform_method
    {
        static constexpr auto value = [] () noexcept
        {};
    };

    template<>
    struct store_uniform_method<glsl_typeid::vec2i>
    {
        static constexpr auto value = glUniform2i;
    };

    template<>
    struct store_uniform_method<glsl_typeid::vec2f>
    {
        static constexpr auto value = glUniform2f;
    };

    template<>
    struct store_uniform_method<glsl_typeid::vec4f>
    {
        static constexpr auto value = glUniform4f;
    };

    template<glsl_typeid id>
    inline constexpr auto store_uniform_method_v = store_uniform_method<id>::value;

    [[nodiscard]]
    bool test_uniform
    (
        shaders_program_resource program,
        uniform_location location,
        glsl_typeid test_typeid,
        zstring_view test_name
    ) noexcept;

    struct uniform_base
    {
        struct null_type : null_t<uniform_location>
        {
            using base_type = null_t<uniform_location>;

            template<class N>
            [[nodiscard]] constexpr operator N () const noexcept
            {
                static_assert(std::is_base_of_v<uniform_base, N> || std::is_same_v<uniform_location, N>);
                return N{ static_cast<const base_type&>(*this) };
            }
        };

        uniform_location location;
    };

    using invaliduniform_t = null_t<uniform_base>;
    inline constexpr invaliduniform_t invaliduniform{};
    static_assert(uniform_location::invalid == invaliduniform);

    template<glsl_typeid id>
    struct uniform : uniform_base
    {
        using value_view_type = glsl_view_t<id>;

        [[nodiscard]]
        bool test(shaders_program_resource program, zstring_view name) const noexcept
        {
            return test_uniform(program, location, id, name);
        }

        void store(value_view_type view) const
        {
            store_uniform_value(location, view);
        }

        template<class... Types>
        auto store(const Types&... values) const -> decltype(store_uniform_method_v<id>(location_as_int(location), values...))
        {
            return store_uniform_method_v<id>(location_as_int(location), values...);
        }

        [[nodiscard]] 
        static uniform instance(shaders_program_resource program, zstring_view name) noexcept
        {
            const uniform result{ get_uniform_location(program, name) };
            D_ASSERT(result.test(program, name));
            return result;
        }
    };

    using uniform_vec2f_t = uniform<glsl_typeid::vec2f>;
    using uniform_vec4f_t = uniform<glsl_typeid::vec4f>;

    namespace literals
    {
        [[nodiscard]]
        constexpr string_view operator"" _glsl(const GLchar * source, size_t length) noexcept
        {
            return { source, length };
        }
    }
}

namespace gl_literals
{
    using namespace gl::literals;
}

