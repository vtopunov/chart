#include "draw.h"


namespace gl
{
    namespace
    {
        [[nodiscard]]
        buffer_descriptor_t gen_buffer() noexcept
        {
            buffer_descriptor_t d{};
            glGenBuffers(1, &d);
            return d;
        }

        void set_array(const_buffer_view data) noexcept
        {
            glBufferData
            (
                GL_ARRAY_BUFFER,
                narrow_cast<GLsizeiptr>(std::size(data)),
                std::data(data),
                GL_STATIC_DRAW
            );
        }
    }

    void set_vertex_pointer
    (
        span<const attribute_location> attributes,
        span<const vertex_attribute_profile> attribute_profiles,
        size_t stride,
        const void* p
    ) noexcept
    {
        D_ASSERT(attributes.size() <= attribute_profiles.size());

        const auto i_stride_bytes = narrow_cast<GLsizei>(stride);

        for (size_t i = 0; i < attributes.size(); ++i) [[likely]]
        {
            if (const auto attribute_location = attributes[i]; 
                attribute_location != invalidattribute) [[likely]]
            {
                const auto attribute_location_index = to_underlying(attribute_location);
                const auto [offset_method, tuple_type] = attribute_profiles[i];

                glVertexAttribPointer
                (
                    attribute_location_index,
                    narrow_cast<GLint>(glsl_tuple_size(tuple_type)),
                    to_underlying(glsl_tuple_element_typeid(tuple_type)),
                    GL_FALSE,
                    i_stride_bytes,
                    offset_method(p)
                );

                glEnableVertexAttribArray(attribute_location_index);
            }
        }

    }

    buffer create_buffer(const_buffer_view data) noexcept
    {
        buffer gl_buffer
        {
            resource_construct,
            gen_buffer()
        };

        if (has_error()) [[unlikely]]
        {
            return {};
        }

        gl::bind(gl_buffer);
        if (has_error()) [[unlikely]]
        {
            return {};
        }
        
        set_array(data);
        if (has_error()) [[unlikely]]
        {
            return {};
        }
        
        return gl_buffer;
    }

    void buffer_resource_deleter::operator()(buffer_resource gl_buffer) const noexcept
    {
        glDeleteBuffers(1, &gl_buffer.d);
    }
}
