#pragma once

#include <array>

#include <core/buffer_view.h>

#include <gl/shader.h>
#include <gl/vertex.h>

namespace gl
{
    enum class draw_mode : GLenum
    {
        points = GL_POINTS,
        lines = GL_LINES,
        line_loop = GL_LINE_LOOP,
        line_strip = GL_LINE_STRIP,
        triangles = GL_TRIANGLES,
        triangle_strip = GL_TRIANGLE_STRIP,
        triangle_fan = GL_TRIANGLE_FAN
    };

    inline void draw_arrays(draw_mode mode, ptrdiff_t first, size_t count) noexcept
    {
        glDrawArrays(to_underlying(mode), narrow_cast<GLint>(first), narrow_cast<GLsizei>(count));
    }

    template<class T>
    void draw_elements(draw_mode mode, size_t size, const T* indices) noexcept
    {
        constexpr auto type_id = glsl_typeid_v<std::decay_t<T>>;

        glDrawElements
        (
            to_underlying(mode),
            narrow_cast<GLsizei>(size),
            to_underlying(type_id),
            indices
        );
    }

    template<class T>
    auto draw_elements(draw_mode mode, const T& data) noexcept
        -> decltype(draw_elements(mode, std::size(data), std::data(data)))
    {
        draw_elements(mode, std::size(data), std::data(data));
    }

    using offset_method_t = const void* (*) (const void*);

    struct vertex_attribute_profile
    {
        offset_method_t offset_method;
        glsl_typeid type;
    };

    constexpr const void* zero_offset_method(const void* p) noexcept
    {
        return p;
    }

    template<class Vertex, size_t AttributeIndex>
    struct vertex_attribute_selector
    {
        using const_vertex_type = const Vertex;
        using const_vertex_pointer = const_vertex_type*;

        static constexpr auto attribute_index = AttributeIndex;
        static constexpr const_vertex_pointer vertex_nullptr{ nullptr };

        static constexpr auto typed_offset_method(const_vertex_pointer ptr) noexcept
            -> decltype(get_ptr<attribute_index>(vertex_nullptr))
        {
            return get_ptr<attribute_index>(ptr);
        }

        static constexpr const void* offset_method(const void* p) noexcept
        {
            return typed_offset_method(static_cast<const Vertex*>(p));
        }

        using attribute_pointer = std::decay_t<decltype(typed_offset_method(vertex_nullptr))>;
        using attribute_type = std::decay_t<std::remove_pointer_t<attribute_pointer>>;

        static constexpr vertex_attribute_profile profile
        {
            .offset_method{ offset_method },
            .type{ glsl_typeid_v<attribute_type> }
        };
    };

    template<class Vertex, class AttributeIndices>
    struct indexed_vertex_selector
    {};

    template<class Vertex, size_t... AttributeIndices>
    struct indexed_vertex_selector<Vertex, std::index_sequence<AttributeIndices...>>
    {
        static constexpr std::array profiles
        {
            vertex_attribute_selector<Vertex, AttributeIndices>::profile...
        };
    };

    template<class Vertex>
    struct vertex_selector
    {
        static constexpr std::array profiles
        {
            vertex_attribute_profile
            {
                .offset_method{ zero_offset_method },
                .type{ glsl_typeid_v<Vertex> }
            }
        };
    };

    template <class... Attributes>
    struct vertex_selector<vertex<Attributes...>>
        : indexed_vertex_selector<vertex<Attributes...>, std::index_sequence_for<Attributes...>>
    {};

    void set_vertex_pointer
    (
        span<const attribute_location> attributes,
        span<const vertex_attribute_profile> attribute_profiles,
        size_t stride,
        const void* p
    ) noexcept;

    template<class Vertex>
    constexpr auto& vertex_profiles_v = vertex_selector<std::decay_t<Vertex>>::profiles;

    template<class Vertex>
    void set_vertex_pointer(span<const attribute_location> attributes, const Vertex* data) noexcept
    {
        constexpr auto& profiles = vertex_profiles_v<Vertex>;
        set_vertex_pointer(attributes, profiles, sizeof(Vertex), data);
    }

    template<class Vertex>
    void set_vertex_pointer(attribute_location attribute, const Vertex* data) noexcept
    {
        set_vertex_pointer(span<attribute_location>{std::addressof(attribute), 1_uz}, data);
    }

    using buffer_descriptor_t = GLuint;

    struct buffer_resource
    {
        buffer_descriptor_t d;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!d;
        }
    };

    struct buffer_resource_deleter
    {
        void operator () (buffer_resource buffer) const noexcept;
    };

    enum class buffer_target : GLenum
    {
        array_buffer = GL_ARRAY_BUFFER
    };

    inline void bind_buffer(buffer_target target, buffer_resource buffer) noexcept
    {
        glBindBuffer(to_underlying(target), buffer.d);
    }

    template<buffer_target Target>
    struct specialized_buffer_resource : buffer_resource
    {
        static constexpr auto target = Target;
    };

    using buffer_resource_t = specialized_buffer_resource<buffer_target::array_buffer>;

    using buffer = unique_resource<buffer_resource_t, buffer_resource_deleter>;

    inline void bind(buffer_resource_t resource) noexcept
    {
        gl::bind_buffer(resource.target, resource);
    }

    [[nodiscard]]
    buffer create_buffer(const_buffer_view data) noexcept;

    template<class Vertex>
    void set_vertex_buffer(span<const attribute_location> attributes, buffer_resource_t buffer) noexcept
    {
        bind(buffer);
        gl::set_vertex_pointer<Vertex>(attributes, nullptr);
    }

    struct vertex_buffer_user
    {
        size_t size;

        void draw(draw_mode mode, ptrdiff_t off = {}) const noexcept
        {
            draw_arrays(mode, off, size);
        }
    };

    template<class Vertex>
    class vertex_buffer
    {
    public:
        constexpr vertex_buffer() noexcept = default;

        vertex_buffer(span<const Vertex> vertexes) noexcept
            : bo_{ create_buffer(vertexes) }
            , size_{ vertexes.size() }
        {}

        D_DISABLE_COPY(vertex_buffer);

        constexpr vertex_buffer(vertex_buffer&& vb) noexcept
            : bo_{ std::move(vb.bo_) }
            , size_{ std::exchange(vb.size_, 0_uz) }
        {}

        constexpr vertex_buffer& operator = (vertex_buffer&& vb) noexcept
        {
            swap(vb);
            return *this;
        }

        constexpr void swap(vertex_buffer& vb) noexcept
        {
            bo_.swap(vb.bo_);
            std::swap(size_, vb.size_);
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!bo_;
        }

        [[nodiscard]]
        constexpr size_t size() const noexcept
        {
            return size_;
        }

        vertex_buffer_user bind(span<const attribute_location> attributes) const noexcept
        {
            gl::set_vertex_buffer<Vertex>(attributes, bo_);
            return { size_ };
        }

        vertex_buffer_user bind(attribute_location attribute) const noexcept
        {
            bind(span<attribute_location>{std::addressof(attribute), 1_uz});
            return { size_ };
        }

    private:
        gl::buffer bo_;
        size_t size_{ 0_uz };
    };

    template <class T, size_t Extent>
    vertex_buffer(T(&)[Extent])->vertex_buffer<std::remove_cv_t<T>>;

    template <class Rng>
    vertex_buffer(Rng&)->vertex_buffer<std::remove_cv_t<typename Rng::value_type>>;

    template <class Rng>
    vertex_buffer(const Rng&)->vertex_buffer<std::remove_cv_t<typename Rng::value_type>>;
}

