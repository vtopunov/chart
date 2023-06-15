#pragma once

#include <core/ordered_overload.h>
#include <core/member_detector.h>
#include <core/limits.h>


namespace private_detail_null_compare
{
    namespace private_private_detail_null_compare
    {
        template<class T>
        constexpr bool eq_null(const T& value) noexcept;

        template<class T>
        constexpr bool not_eq_null(const T& value) noexcept;
    }

    template<class T>
    using decl_eq_op_t = decltype(std::declval<const T&>() == std::declval<const T&>());

    template<class T>
    using decl_not_eq_op_t = decltype(std::declval<const T&>() != std::declval<const T&>());

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<is_detected_v<decl_eq_op_t, T>, bool> eq_null(const T& value) noexcept
    {
        return private_private_detail_null_compare::eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<is_detected_v<decl_not_eq_op_t, T>, bool> not_eq_null(const T& value) noexcept
    {
        return private_private_detail_null_compare::not_eq_null(value);
    }
}

namespace private_detail_has_value
{
    using private_detail_null_compare::eq_null;
    using private_detail_null_compare::not_eq_null;
    using namespace ordered_overload;

    template<class T>
    [[nodiscard]] constexpr auto has_value(const T& value, _order<_2>) noexcept
        -> decltype(!eq_null(value))
    {
        return !eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto has_value(const T& value, _order<_1>) noexcept
        -> decltype(not_eq_null(value))
    {
        return not_eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto has_value(const T& value, _order<_0>) noexcept
        -> decltype(!!value)
    {
        return !!value;
    }

    template<class T>
    [[nodiscard]] constexpr auto has_value(const T& value) noexcept
        -> decltype(has_value(value, _start))
    {
        return has_value(value, _start);
    }
}

namespace private_detail_is_null
{
    using private_detail_null_compare::eq_null;
    using private_detail_null_compare::not_eq_null;
    using namespace ordered_overload;

    template<class T>
    [[nodiscard]] constexpr auto is_null(const T& value, _order<_2>) noexcept
        -> decltype(!not_eq_null(value))
    {
        return !not_eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto is_null(const T& value, _order<_1>) noexcept
        -> decltype(eq_null(value))
    {
        return eq_null(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto is_null(const T& value, _order<_0>) noexcept
        -> decltype(!value)
    {
        return !value;
    }

    template<class T>
    [[nodiscard]] constexpr auto is_null(const T& value) noexcept
        -> decltype(is_null(value, _start))
    {
        return is_null(value, _start);
    }
}

namespace private_detail_enum_instance
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

namespace private_detail_null_instance
{
    template<class T>
    struct null_instance
    {
        static_assert(!std::is_reference_v<T>);

        [[nodiscard]]
        constexpr operator T () const noexcept
        {
            if constexpr (std::is_enum_v<T>)
            {
                return private_detail_enum_instance::enum_instance<T>();
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
}

namespace private_detail_null_type
{  
    template<class T>
    using null_instance_t = private_detail_null_instance::null_instance<T>;

    template<class T>
    using remove_qualifiers_t = std::remove_cvref_t<T>;

    template<class T>
    using decl_null_type_t = typename T::null_type;

    template<class T>
    struct null_type_select2
    {
        using type = detected_or_t<null_instance_t<T>, decl_null_type_t, T>;
    };

    template<>
    struct null_type_select2<std::nullptr_t>
    {
        using type = std::nullptr_t;
    };

    template<class T>
    struct null_type_select2<T*>
    {
        using type = std::nullptr_t;
    };

    template<class T>
    struct null_type_select1 : null_type_select2<remove_qualifiers_t<T>>
    {};

    template<class T>
    struct null_type_select1<null_instance_t<T>> : null_type_select1<remove_qualifiers_t<T>>
    {};

    template<class T>
    struct null_type_select0 : null_type_select1<remove_qualifiers_t<T>>
    {};

    template<class T>
    using null_t = typename null_type_select0<T>::type;

    template<class T>
    constexpr null_t<T> null_v{};
}

namespace private_detail_null_compare
{
    namespace private_private_detail_null_compare
    {
        template<class T>
        [[nodiscard]] constexpr bool eq_null(const T& value) noexcept
        {
            using private_detail_null_type::null_v;
            return value == static_cast<T>(null_v<T>);
        }

        template<class T>
        [[nodiscard]] constexpr bool not_eq_null(const T& value) noexcept
        {
            using private_detail_null_type::null_v;
            return value != static_cast<T>(null_v<T>);
        }
    }
}

namespace private_detail_is_nullable
{
    template<class T>
    using has_value_t = decltype(private_detail_has_value::has_value(std::declval<T&>()));

    template<class T>
    using is_nullable = is_detected<has_value_t, T>;

    template<class T>
    constexpr bool is_nullable_v = is_nullable<T>::value;
}

namespace private_detail_null_instance
{
    using private_detail_null_type::null_t;

    template<class T>
    constexpr auto has_cmp_with_null_v = std::conjunction_v<
        private_detail_is_nullable::is_nullable<T>,
        std::is_class<null_t<T>>
    >;

    template<class T, std::enable_if_t<has_cmp_with_null_v<T>, int> = 0>
    [[nodiscard]] constexpr decltype(auto) operator != (const T& value, null_t<T>) noexcept
    {
        return private_detail_has_value::has_value(value);
    }

    template<class T, std::enable_if_t<has_cmp_with_null_v<T>, int> = 0>
    [[nodiscard]] constexpr decltype(auto) operator != (null_t<T>, const T& value) noexcept
    {
        return private_detail_has_value::has_value(value);
    }
 
    template<class T, std::enable_if_t<has_cmp_with_null_v<T>, int> = 0>
    [[nodiscard]] constexpr decltype(auto) operator == (const T& value, null_t<T>) noexcept
    {
        return private_detail_is_null::is_null(value);
    }

    template<class T, std::enable_if_t<has_cmp_with_null_v<T>, int> = 0>
    [[nodiscard]] constexpr decltype(auto) operator == (null_t<T>, const T& value) noexcept
    {
        return private_detail_is_null::is_null(value);
    }
}


using private_detail_null_type::null_t;
using private_detail_null_type::null_v;
using private_detail_has_value::has_value;
using private_detail_is_null::is_null;
using private_detail_is_nullable::is_nullable_v;
