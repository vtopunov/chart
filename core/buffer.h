#pragma once

#include "span.h"

class buffer
{
public:
    virtual ~buffer() noexcept = default;

    virtual void free() noexcept = 0;

    virtual void shrink() noexcept = 0;

    virtual size_t size() const noexcept = 0;

    template<class T>
    span<T> get(size_t size) noexcept
    {
        using pointer = typename span<T>::pointer;
        const auto bytes = size * sizeof(T);
        D_ASSERT(size == bytes / sizeof(T));
        return { static_cast<pointer>(alloc(bytes)), size };
    }

    static buffer& default_instance() noexcept;

private:
    virtual void* alloc(size_t size) noexcept = 0;
};