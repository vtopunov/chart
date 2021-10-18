#pragma once

#include <type_traits>

#include <core/member_detector.h>

namespace private_detail_null_instance
{
    template<class T>
    using has_null_t = typename T::null_type;

    template<class T>
    struct null_instance
    {
        template<class N> [[nodiscard]]
        constexpr operator N () const noexcept
        {
            return N{};
        }
    };

    template<class T>
    struct select
    {
        using type = detected_or_t<null_instance<T>, has_null_t, T>;
    };

    template<>
    struct select<std::nullptr_t>
    {
        using type = std::nullptr_t;
    };

    template<class T>
    struct select<T*>
    {
        using type = std::nullptr_t;
    };

    template<class T>
    struct select<null_instance<T>>
    {
        using type = null_instance<std::decay_t<T>>;
    };
}

template<class T>
using null_t = typename private_detail_null_instance::select<std::decay_t<T>>::type;

namespace private_detail_validate
{
    struct _2
    {};

    struct _1 : _2
    {};

    struct _0 : _1
    {};

    template<class T>
    using _order = const T*const;

    inline constexpr _order<_0> _start{ nullptr };

    template<class T> [[nodiscard]]
    constexpr auto has_value(const T& value, _order<_2>) noexcept
        -> decltype( !( std::declval<const T&>() == std::declval<const T&>() ) )
    {
        return !(value == T{ null_t<T>{} });
    }

    template<class T> [[nodiscard]]
    constexpr auto has_value(const T& value, _order<_1>) noexcept
        -> decltype( std::declval<const T&>() != std::declval<const T&>() )
    {
        return value != T{ null_t<T>{} };
    }

    template<class T> [[nodiscard]]
    constexpr auto has_value(const T& value, _order<_0>) noexcept
        -> decltype( !!std::declval<const T&>() )
    {
        return !!value;
    }

    template<class T> [[nodiscard]]
    constexpr auto has_value(const T& value) noexcept
        -> decltype(has_value(value, _start) )
    {
        return has_value(value, _start);
    }
}

template<class T> [[nodiscard]]
constexpr auto has_value(const T& value) noexcept -> decltype( private_detail_validate::has_value(value) )
{
    return private_detail_validate::has_value(value);
}

template<class T> [[nodiscard]]
constexpr auto operator != (const T& value, null_t<T>) noexcept -> decltype( has_value(value) )
{
    return has_value(value);
}

template<class T> [[nodiscard]]
constexpr auto operator != (null_t<T>, const T& value) noexcept -> decltype( has_value(value) )
{
    return has_value(value);
}

template<class T> [[nodiscard]]
constexpr auto operator == (const T& value, null_t<T>) noexcept -> decltype( !has_value(value) )
{
    return !has_value(value);
}

template<class T> [[nodiscard]]
constexpr auto operator == (null_t<T>, const T& value) noexcept -> decltype( !has_value(value) )
{
    return !has_value(value);
}