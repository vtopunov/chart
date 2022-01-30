#pragma once

#include <core/colorfwd.h>

namespace px
{
    template<class T>
    struct pixline
    {
        using pointer = T*;
        using const_pointer = std::add_const_t<T>*;

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

    using pix8line_t = pixline<u8tint_t>;
    using const_pix8line_t = pixline<const u8tint_t>;
}

using px::pixline;
using px::pix8line_t;
using px::const_pix8line_t;