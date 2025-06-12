#pragma once

#include <core/small_vector.h>


namespace private_detail_static_vector
{
    template<class T>
    struct dummy_buffer
    {
        using size_type = size_t;

        constexpr dummy_buffer() noexcept = default;

        D_DEFAULT_ONLYMOVE_CA(dummy_buffer);

        constexpr explicit dummy_buffer(size_type) noexcept
        {}

        [[nodiscard]]
        constexpr T* data() const noexcept
        {
            return nullptr;
        }

        [[nodiscard]]
        constexpr size_type size() const noexcept
        {
            return 0u;
        }

        constexpr explicit operator bool() const noexcept
        {
            return false;
        }

        constexpr void swap(dummy_buffer&) noexcept
        {}

        [[nodiscard]]
        static constexpr size_type good_size(size_type size) noexcept
        {
            return size;
        }
    };
}

template<class T, size_t N>
using static_vector = small_vector<T, N, private_detail_static_vector::dummy_buffer<T>>;
