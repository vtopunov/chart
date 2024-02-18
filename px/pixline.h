#pragma once

#include <px/fwd.h>


namespace px
{
    template<class T>
    struct pixline
    {
        using pointer = T*;
        using const_pointer = const T*;

        pointer position;
        size_t size;

        [[nodiscard]]
        constexpr bool operator==(const pixline& right) const noexcept
        {
            return position == right.position;
        }

        [[nodiscard]]
        constexpr bool operator!=(const pixline& right) const noexcept
        {
            return position != right.position;
        }

        [[nodiscard]]
        constexpr bool operator==(const_pointer right) const noexcept
        {
            return position == right;
        }

        [[nodiscard]]
        constexpr bool operator!=(const_pointer right) const noexcept
        {
            return position != right;
        }

        constexpr pixline& operator++() noexcept
        {
            position += size;
            return *this;
        }

        [[nodiscard]]
        constexpr pointer operator*() const noexcept
        {
            return position;
        }
    };

    template<class T> 
    [[nodiscard]] constexpr bool operator==(const T* left, const pixline<T>& right) noexcept
    {
        return left == right.position;
    }

    template<class T> 
    [[nodiscard]] constexpr bool operator!=(const T* left, const pixline<T>& right) noexcept
    {
        return left != right.position;
    }

    using pix8line = pixline<pix8_t>;
    using const_pix8line = pixline<const pix8_t>;
}

using px::pixline;
using px::pix8line;
using px::const_pix8line;