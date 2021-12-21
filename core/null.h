#pragma once

#include <type_traits>
#include <limits>

#include <core/utility.h>
#include <core/member_detector.h>


namespace private_detail_null_instance
{
    using namespace ordered_overload;


    template<class T>
    [[nodiscard]] constexpr T enum_instance(_overload<T>, _order<_2>) noexcept
    {
        return {};
    }

    template<class T>
    [[nodiscard]] constexpr auto enum_instance(_overload<T>, _order<_1>) noexcept -> decltype(T::null)
    {
        return T::null;
    }

    template<class T>
    [[nodiscard]] constexpr auto enum_instance(_overload<T>, _order<_0>) noexcept -> decltype(T::invalid)
    {
        return T::invalid;
    }

    template<class T>
    [[nodiscard]] constexpr T enum_instance() noexcept
    {
        return enum_instance(_overload_v<T>, _start);
    }
}

template<class T>
using decl_null_type_t = typename T::null_type;

template<class T>
struct null_instance
{
    static_assert(!std::is_reference_v<T>);

    [[nodiscard]]
    constexpr operator T () const noexcept
    {
        if constexpr (std::is_enum_v<T>)
        {
            return private_detail_null_instance::enum_instance<T>();
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            return numeric_nan_v<T>;
        }
        else
        {
            return {};
        }
    }
};

template<class T>
struct null_type
{
    using type = detected_or_t<null_instance<T>, decl_null_type_t, T>;
};

template<>
struct null_type<std::nullptr_t>
{
    using type = std::nullptr_t;
};

template<class T>
struct null_type<T*>
{
    using type = std::nullptr_t;
};

template<class T>
struct null_type<null_instance<T>>
{
    using type = null_instance<std::remove_cvref_t<T>>;
};

template<class T>
struct null_type<T&> : null_type<T>
{};

template<class T>
struct null_type<T&&> : null_type<T>
{};

template<class T>
struct null_type<const T> : null_type<T>
{};

template<class T>
using null_t = typename null_type<T>::type;

template<class T>
inline constexpr null_t<T> null_v{};


namespace private_detail_null_compare
{
    template<class T>
    [[nodiscard]] constexpr auto eq_null(const T& value) noexcept -> decltype(value == value)
    {
        return value == static_cast<T>(null_v<T>);
    }

    template<class T>
    [[nodiscard]] constexpr auto not_eq_null(const T& value) noexcept -> decltype(value != value)
    {
        return value != static_cast<T>(null_v<T>);
    }
}


namespace private_detail_has_value
{
    using namespace private_detail_null_compare;
    using namespace ordered_overload;


    template<class T>
    [[nodiscard]] constexpr auto has_value(const T& value, _order<_2>) noexcept -> decltype(!eq_null(value))
    {
        return !eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto has_value(const T& value, _order<_1>) noexcept -> decltype(not_eq_null(value))
    {
        return not_eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto has_value(const T& value, _order<_0>) noexcept -> decltype(!!value)
    {
        return !!value;
    }

    template<class T>
    [[nodiscard]] constexpr auto has_value(const T& value) noexcept -> decltype(has_value(value, _start))
    {
        return has_value(value, _start);
    }
}


namespace private_detail_is_null
{
    using namespace private_detail_null_compare;
    using namespace ordered_overload;

    template<class T>
    [[nodiscard]] constexpr auto is_null(const T& value, _order<_2>) noexcept -> decltype(!not_eq_null(value))
    {
        return !not_eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto is_null(const T& value, _order<_1>) noexcept -> decltype(eq_null(value))
    {
        return eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto is_null(const T& value, _order<_0>) noexcept -> decltype(!value)
    {
        return !value;
    }

    template<class T>
    [[nodiscard]] constexpr auto is_null(const T& value) noexcept -> decltype(is_null(value, _start))
    {
        return is_null(value, _start);
    }
}


template<class T> [[nodiscard]]
constexpr auto has_value(const T& value) noexcept -> decltype(private_detail_has_value::has_value(value))
{
    return private_detail_has_value::has_value(value);
}

template<class T> [[nodiscard]]
constexpr auto is_null(const T& value) noexcept -> decltype(private_detail_is_null::is_null(value))
{
    return private_detail_is_null::is_null(value);
}

template<class T> [[nodiscard]]
constexpr auto operator != (const T& value, null_t<T>) noexcept -> decltype(has_value(value))
{
    return has_value(value);
}

template<class T> [[nodiscard]]
constexpr auto operator != (null_t<T>, const T& value) noexcept -> decltype(has_value(value))
{
    return has_value(value);
}

template<class T> [[nodiscard]]
constexpr auto operator == (const T& value, null_t<T>) noexcept -> decltype(is_null(value))
{
    return is_null(value);
}

template<class T> [[nodiscard]]
constexpr auto operator == (null_t<T>, const T& value) noexcept -> decltype(is_null(value))
{
    return is_null(value);
}