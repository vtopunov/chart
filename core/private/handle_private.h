#pragma once

#include <core/member_detector.h>

namespace private_handle
{
    template<class T>
    using has_replace_owwer_t = decltype(std::declval<T>().replace_owwer(nullptr), 0);

    template<class T>
    using has_is_valid_t = decltype(std::declval<T>().is_valid());

    template<class T>
    using has_view_t = decltype(std::declval<T>().view());

    template<class T>
    constexpr bool has_replace_owwer_v = is_detected_v<has_replace_owwer_t, T>;

    template<class T>
    constexpr bool has_is_valid_v = is_detected_v<has_is_valid_t, T>;

    template<class T>
    constexpr bool has_view_v = is_detected_v<has_view_t, T>;

    template<class T>
    constexpr decltype(auto) view(const T& handle) noexcept
    {
        if constexpr (has_view_v<T>)
        {
            return handle.view();
        }
        else
        {
            constexpr struct {} private_view;
            return private_view;
        }
    }

    template<class T>
    using view_type_t = decltype(private_handle::view(std::declval<T>()));

    template<class T>
    constexpr bool is_valid(const T& handle) noexcept
    {
        if constexpr (has_is_valid_v<T>)
        {
            return handle.is_valid();
        }
        else 
        {
            return true;
        }
    }
}
