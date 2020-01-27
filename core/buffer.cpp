#include "buffer.h"

namespace
{
    constexpr unsigned long long operator "" _Kb(unsigned long long n) noexcept
    {
        return n * 1024ULL;
    }

    constexpr unsigned long long operator "" _Mb(unsigned long long n) noexcept
    {
        return n * 1024_Kb;
    }

    constexpr size_t max_size_for_realocate = 512_Mb;
    constexpr size_t min_size_for_realocate = 256_Mb;
}

buffer& buffer::default_instance() noexcept
{
    class default_buffer final : public buffer
    {
        void* alloc(size_t size) noexcept final
        {
            shrink_if_need(size);
            buffer.reserve(size);
            return buffer.data();
        }

        size_t size() const noexcept final
        {
            return buffer.capacity();
        }

        void free() noexcept final
        {
            buffer.clear();
            buffer.shrink_to_fit();
        }

        void shrink() noexcept final
        {
            shrink_if_need(0);
        }

        void shrink_if_need(size_t size) noexcept
        {
            const auto capacity = buffer.capacity();

            if (capacity > max_size_for_realocate && size < min_size_for_realocate)
            {
                free();
            }
        }

        std::string buffer;
    };

    static default_buffer buffer;
    return buffer;
}
