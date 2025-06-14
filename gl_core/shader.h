#pragma once

#include <string_view>

#include <core/resource.h>
#include <core/zstring_view.h>

#include <px/pxf.h>

#include <gl_core/glsl_typeid.h>


namespace gl
{
    using source_view = std::string_view;

    [[nodiscard]]
    inline bool is_correct() noexcept
    {
        return GL_NO_ERROR == glGetError();
    }

    [[nodiscard]]
    inline bool has_error() noexcept
    {
        return !is_correct();
    }

    enum class shader_resource : GLuint
    {};

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

    template<shader_type TypeId>
    struct typed_source_view
    {
        static constexpr auto type_id = TypeId;
        source_view source;
    };

    using fragment_source_view = typed_source_view<shader_type::fragment>;
    using vertex_source_view = typed_source_view<shader_type::vertex>;

    [[nodiscard]]
    shader create_shader(shader_type type) noexcept;

    enum class program_resource : GLuint
    {};

    void attach_shader(program_resource program, shader_resource shader) noexcept;

    [[nodiscard]]
    bool compile(program_resource program, source_view source, shader_type type) noexcept;

    template<shader_type type>
    [[nodiscard]] bool compile(program_resource program, typed_source_view<type> source) noexcept
    {
        return compile(program, source.source, type);
    }

    [[nodiscard]]
    bool link(program_resource program) noexcept;

    inline void use(program_resource program) noexcept
    {
        glUseProgram(to_underlying(program));
    }

    program_resource current_program() noexcept;

    struct program_resource_deleter
    {
        void operator () (program_resource program) const noexcept;
    };

    using program = unique_resource<program_resource, program_resource_deleter>;

    [[nodiscard]]
    program create_program() noexcept;

    [[nodiscard]]
    program create_program(vertex_source_view vertex, fragment_source_view fragment) noexcept;

    using location_index_t = GLuint;
    static_assert(std::is_unsigned_v<location_index_t>);
    constexpr auto invalid_location_index = numeric_max_v<location_index_t>;

    enum class attribute_location : location_index_t
    {
        invalid = invalid_location_index
    };

    using invalidattribute_t = null_t<attribute_location>;
    constexpr invalidattribute_t invalidattribute{};
    static_assert(attribute_location::invalid == invalidattribute);

    [[nodiscard]]
    attribute_location get_attribute_location(program_resource program, zstring_view name) noexcept;

    template<class... Names>
    [[nodiscard]] std::array<attribute_location, sizeof...(Names)> get_attribute_locations
    (
        program_resource program,
        const Names&... names
    ) noexcept
    {
        return { get_attribute_location(program, names)... };
    }

    enum class uniform_location : location_index_t
    {
        invalid = invalid_location_index
    };

    using location_int_t = std::make_signed_t<location_index_t>;
    static_assert(std::is_same_v<location_int_t, GLint>);

    [[nodiscard]]
    constexpr location_int_t location_as_int(uniform_location location) noexcept
    {
        return narrow<location_int_t>(location);
    }

    [[nodiscard]]
    uniform_location get_uniform_location(program_resource program, zstring_view name) noexcept;

    inline void store_uniform_view(uniform_location u, GLint value) noexcept
    {
        glUniform1i(location_as_int(u), value);
    }

    inline void store_uniform_view(uniform_location u, GLfloat value) noexcept
    {
        glUniform1f(location_as_int(u), value);
    }

    inline void store_uniform_view(uniform_location u, const_span2i value) noexcept
    {
        glUniform2iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_view(uniform_location u, const_span2f value) noexcept
    {
        glUniform2fv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_view(uniform_location u, const_span3i value) noexcept
    {
        glUniform3iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_view(uniform_location u, const_span3f value) noexcept
    {
        glUniform3fv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_view(uniform_location u, const_span4i value) noexcept
    {
        glUniform4iv(location_as_int(u), 1, std::data(value));
    }

    inline void store_uniform_view(uniform_location u, const_span4f value) noexcept
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
        program_resource program,
        uniform_location location,
        glsl_typeid test_typeid,
        std::string_view test_name
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
        using element_type = glsl_tuple_element_type_t<type_id>;

        [[nodiscard]]
        bool test(program_resource program, std::string_view name) const noexcept
        {
            return test_uniform(program, location, type_id, name);
        }

        void store(value_view_type view) const noexcept
        {
            store_uniform_view(location, view);
        }

        template<class... Types>
        auto store(const Types&... values) const 
            -> decltype(store_uniform_method_v<type_id>(location_as_int(location), px::narrow_px<element_type>(values)...))
        {
            return store_uniform_method_v<type_id>(location_as_int(location), px::narrow_px<element_type>(values)...);
        }

        template<class Vec>
        auto store(const Vec& v) const -> decltype(store(as_vec2(v)._0, as_vec2(v)._1))
        {
            return store(v._0, v._1);
        }

        [[nodiscard]]
        static uniform instance(program_resource program, zstring_view name) noexcept
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
        constexpr source_view operator ""_glsl(const char* source, size_t length) noexcept
        {
            return { source, length };
        }

        [[nodiscard]]
        constexpr vertex_source_view operator ""_vert_glsl(const char* source, size_t length) noexcept
        {
            return { .source{ source, length } };
        }

        [[nodiscard]]
        constexpr fragment_source_view operator ""_frag_glsl(const char* source, size_t length) noexcept
        {
            return { .source{ source, length } };
        }
    }
}

using namespace gl::shader_literals;

