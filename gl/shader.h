#pragma once

#include <string_view>

#include <core/assert.h>
#include <core/resource.h>
#include <core/zstring_view.h>

#include <gl/glsl_typeid.h>


namespace gl
{
    inline bool is_correct() noexcept
    {
        return GL_NO_ERROR == glGetError();
    }

    inline bool has_error() noexcept
    {
        return !is_correct();
    }


    using zstring_view = basic_zstring_view<GLchar>;
    using string_view = std::basic_string_view<GLchar>;
    using source_view = string_view;

    enum class shader_resource : GLuint
    {
        null
    };

    void set_source(shader_resource shader, source_view source) noexcept;

    bool compile(shader_resource shader) noexcept;

    struct shader_resource_deleter
    {
        void operator () (shader_resource shader) const noexcept;
    };

    using shader = unique_resource<shader_resource, shader_resource_deleter>;

    enum class shader_type : GLenum
    {
        fragment = GL_FRAGMENT_SHADER,
        vertex = GL_VERTEX_SHADER
    };

    [[nodiscard]]
    shader create_shader(shader_type type) noexcept;

    enum class shaders_program_resource : GLuint
    {
        null
    };

    void attach_shader(shaders_program_resource program, shader_resource shader) noexcept;

    bool compile(shaders_program_resource program, source_view source, shader_type type) noexcept;

    bool link(shaders_program_resource program) noexcept;

    inline void use(shaders_program_resource program) noexcept
    {
        glUseProgram(to_underlying(program));
    }

    struct shaders_program_resource_deleter
    {
        void operator () (shaders_program_resource program) const noexcept;
    };

    using shaders_program = unique_resource<shaders_program_resource, shaders_program_resource_deleter>;

    [[nodiscard]]
    shaders_program create_shaders_program() noexcept;

    [[nodiscard]]
    shaders_program create_shaders_program(source_view vertex, source_view fragment) noexcept;

    using location_numer_t = GLuint;
    static_assert(std::is_unsigned_v<location_numer_t>);
    constexpr auto invalid_location_number = numeric_max_v<location_numer_t>;

    enum class attribute_location : location_numer_t
    {
        invalid = invalid_location_number
    };

    using invalidattribute_t = null_t<attribute_location>;   
    constexpr invalidattribute_t invalidattribute{};
    static_assert(attribute_location::invalid == invalidattribute);

    [[nodiscard]]
    attribute_location get_attribute_location(shaders_program_resource program, zstring_view name) noexcept;

    template<class... Names> [[nodiscard]]
    std::array<attribute_location, sizeof...(Names)>  get_attribute_locations(shaders_program_resource program, const Names&... names) noexcept
    {
         return { get_attribute_location(program, names)... };
    }

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

    inline void store_uniform_value(uniform_location u, const_span2i value) noexcept
    {
        glUniform2iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span2f value) noexcept
    {
        glUniform2fv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span3i value) noexcept
    {
        glUniform3iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span3f value) noexcept
    {
        glUniform3fv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span4i value) noexcept
    {
        glUniform4iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_value(uniform_location u, const_span4f value) noexcept
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
    constexpr auto store_uniform_method_v = store_uniform_method<id>::value;

    [[nodiscard]]
    bool test_uniform
    (
        shaders_program_resource program,
        uniform_location location,
        glsl_typeid test_typeid,
        string_view test_name
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
    constexpr invaliduniform_t invaliduniform{};
    static_assert(uniform_location::invalid == invaliduniform);

    template<glsl_typeid TypeId>
    struct uniform : uniform_base
    {
        static constexpr glsl_typeid type_id{ TypeId };
        using value_view_type = glsl_view_t<type_id>;
       
#if D_IS_DEBUG
        static constexpr int DEBUG_STORED__{ 1 << 0 };
        static constexpr int DEBUG_RO__{ 1 << 1 };

        mutable int debug_flags__{ 0 };

        constexpr void __debug_store() const noexcept
        {
            D_ASSERT(!(debug_flags__ & DEBUG_RO__));
            debug_flags__ |= DEBUG_STORED__;
        }

        constexpr void __debug_set_ro() const noexcept
        {
            D_ASSERT(__debug_is_stored());
            debug_flags__ |= DEBUG_RO__;
        }

        constexpr bool __debug_is_stored() const noexcept
        {
            return !!(debug_flags__ & DEBUG_STORED__);
        }
#endif

        [[nodiscard]]
        bool test(shaders_program_resource program, string_view name) const noexcept
        {
            return test_uniform(program, location, type_id, name);
        }

        void store(value_view_type view) const
        {
#if D_IS_DEBUG
            __debug_store();
#endif

            store_uniform_value(location, view);
        }

        template<class... Types>
        auto store(const Types&... values) const -> decltype(store_uniform_method_v<type_id>(location_as_int(location), values...))
        {
#if D_IS_DEBUG
           __debug_store();
#endif

            return store_uniform_method_v<type_id>(location_as_int(location), values...);
        }

        [[nodiscard]] 
        static uniform instance(shaders_program_resource program, zstring_view name) noexcept
        {
            const uniform result{ get_uniform_location(program, name) };
            D_ASSERT(result.test(program, name.c_str()));
            return result;
        }
    };

    using uniform_vec2f = uniform<glsl_typeid::vec2f>;
    using uniform_vec4f = uniform<glsl_typeid::vec4f>;

    namespace shader_literals
    {
        [[nodiscard]]
        constexpr source_view operator"" _glsl(const GLchar * source, size_t length) noexcept
        {
            return { source, length };
        }
    }
}

using namespace gl::shader_literals;

