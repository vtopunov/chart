#include "draw.h"


namespace gl
{
    namespace
    {
        [[nodiscard]]
        buffer gen_buffer() noexcept
        {
            buffer_descriptor_t d{};
            glGenBuffers(1, &d);
            return buffer{ d };
        }

        void set_array(const_byte_buffer_view data) noexcept
        {
            glBufferData
            (
                GL_ARRAY_BUFFER,
                narrow<GLsizeiptr>(std::size(data)),
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
        D_ASSERT_OR_ASSUME(attributes.size() <= attribute_profiles.size());

        const auto i_stride_bytes = narrow<GLsizei>(stride);

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
                    narrow<GLint>(glsl_tuple_size(tuple_type)),
                    to_underlying(glsl_tuple_element_typeid(tuple_type)),
                    GL_FALSE,
                    i_stride_bytes,
                    offset_method(p)
                );

                glEnableVertexAttribArray(attribute_location_index);
            }
        }

    }

    buffer create_buffer(const_byte_buffer_view data) noexcept
    {
        if (auto gl_buffer = gen_buffer()) [[likely]]
        {
            gl::bind(gl_buffer);
            if (is_correct()) [[likely]]
            {
                set_array(data);
                if (is_correct()) [[likely]]
                {
                    return gl_buffer;
                }
            }
        }

        return {};
    }

    void buffer_resource_deleter::operator()(buffer_resource gl_buffer) const noexcept
    {
        glDeleteBuffers(1, &gl_buffer.d);
    }
}
