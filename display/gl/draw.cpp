#include "draw.h"

namespace display
{
    namespace gl
    {
        namespace
        {
            buffer_descriptor_t gen_buffer() noexcept
            {
                buffer_descriptor_t d{};
                glGenBuffers(1, &d);
                return d;
            }

            void write(buffer_resource_t resource, const_buffer_view data) noexcept
            {
                glBufferData
                (
                    to_underlying(resource.target),
                    narrow_cast<GLsizeiptr>(std::size(data)),
                    std::data(data),
                    to_underlying(resource.usage)
                );
            }
        }

        void set_vertex_pointer
        (
            std::span<const attribute_location> attributes,
            std::span<const vertex_attribute_profile> attribute_profiles,
            size_t stride,
            const void* p
        ) noexcept
        {
            D_ASSERT(attributes.size() <= attribute_profiles.size());

            const auto i_stride_bytes = narrow_cast<GLsizei>(stride);

            for (size_t i = 0; i < attributes.size(); ++i)
            {
                const auto [offset_method, tuple_type] = attribute_profiles[i];
                const auto attribute_location_index = to_underlying(attributes[i]);

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

        buffer_t create_buffer(const_buffer_view data) noexcept
        {
            buffer_t gl_buffer
            {
                resource_construct,
                gen_buffer()
            };

            D_ASSERT(gl_buffer);

            gl_buffer->bind();
  
            write(gl_buffer, data);
   
            return gl_buffer;
        }

        void buffer_resource_deleter::operator()(buffer_resource buffer, resource_destroy_t) const noexcept
        {
            glDeleteBuffers(1, &buffer.d);
        }



    }
}