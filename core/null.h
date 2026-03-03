#pragma once

#include <optional>

#include <core/zero.h>


namespace private_detail_null
{
    template<class T>
    using decl_null_value_type_t = typename T::null_value_type;

    template<class T>
    using remove_null_t = detected_or_t<T, decl_null_value_type_t, T>;

    template<class T>
    using is_null_type = is_detected<decl_null_value_type_t, T>;

    template<class T>
    constexpr bool is_null_type_v = is_null_type<T>::value;

    namespace private_detail_null_instance
    {
        template<class T>
        using decl_invalid_t = decltype(T::invalid);

        template<class T>
        using decl_invalid_fn_t = decltype(T::invalid());

        template<class T, class = void>
        struct null_instance2
        {
            using is_null_constructible_constant_type = std::false_type;
        };

        template<class T>
        struct null_instance2<T, std::enable_if_t<is_zero_constructible_v<T> > > : private_detail_zero::private_detail_zero_type::zero_type<T>
        {
            using is_null_constructible_constant_type = std::true_type;
        };

        template<class T, class = void>
        struct null_instance1 : null_instance2<T>
        {};

        template<class T>
        struct null_instance1<T, std::void_t<decl_invalid_t<T>> >
        {
            using is_null_constructible_constant_type = std::true_type;

            constexpr operator T () const noexcept
            {
                return T::invalid;
            }
        };

        template<class T, class = void>
        struct null_instance0 : null_instance1<T>
        {};

        template<class T>
        struct null_instance0<T, std::void_t<decl_invalid_fn_t<T>> >
        {
            using is_null_constructible_constant_type = std::true_type;

            constexpr operator T () const noexcept
            {
                return T::invalid();
            }
        };

        template<class T>
        struct null_instance : null_instance0<T>
        {
            static_assert(!std::is_reference_v<T>);
            using null_value_type = T;
        };
    }


    namespace private_detail_null_type
    {
        using namespace ordered_overload;

        template<class To, class From>
        using decl_cref_cast_t = decltype(static_cast<const To&>(std::declval<const From&>()));

        template<class To, class From>
        using has_cref_cast = is_detected<decl_cref_cast_t, To, From>;

        template<class T, class = void>
        struct null_type_type1
        {
            using type = private_detail_null_instance::null_instance<T>;
            using is_null_constructible_constant_type = typename type::is_null_constructible_constant_type;
        };

        template<class T>
        struct null_type_type1<T, std::enable_if_t<std::is_constructible_v<T, std::nullopt_t> > >
        {
            using type = std::nullopt_t;
            using is_null_constructible_constant_type = std::true_type;
        };

        template<class T>
        struct null_type_type1<T, std::enable_if_t<std::is_constructible_v<T, nulltype_construct_t> > >
        {
            using type = T;
            using is_null_constructible_constant_type = std::true_type;
        };


        template<class T, class = void>
        struct null_type_type0 : null_type_type1<T>
        {};

        template<class T>
        struct null_type_type0<T, std::enable_if_t<is_pointer_or_nullptr_v<T>>>
        {
            using type = std::nullptr_t;
            using is_null_constructible_constant_type = std::true_type;
        };

        template<class T>
        struct null_type_type0<T, std::void_t<decl_null_type_t<T>> >
        {
            using type = decl_null_type_t<T>;
            static_assert(!std::is_const_v<T>);
            static_assert(!std::is_reference_v<T>);
            static_assert(has_no_unique_address_v<type>);
            using is_null_constructible_constant_type = has_cref_cast<T, type>;
        };

        template<class T>
        using null_type_type = null_type_type0<std::remove_cvref_t<T> >;

        template<class T>
        using is_null_constructible = typename null_type_type<T>::is_null_constructible_constant_type;

        template<class T>
        constexpr bool is_null_constructible_v = is_null_constructible<T>::value;

        template<class T>
        using null_t = typename null_type_type<T>::type;

        template<class NullT>
        struct instance_null_t
        {
            [[nodiscard]] constexpr NullT operator () () const noexcept
            {
                if constexpr (std::is_default_constructible_v<NullT>)
                {
                    return NullT{};
                }
                else if constexpr (std::is_constructible_v<NullT, std::nullopt_t>)
                {
                    return NullT(std::nullopt);
                }
                else
                {
                    static_assert(std::is_constructible_v<NullT, nulltype_construct_t>);
                    return NullT(nulltype_construct);
                }
            }
        };

        template<class NullT>
        constexpr instance_null_t<NullT> instance_null{};

        template<class T>
        constexpr auto null_v = instance_null<null_t<T>>();

        template<size_t N>
        constexpr const void* const p_memzero_v = [] () noexcept
        {
            static constexpr std::byte zero_array[N]{};
            return zero_array;
        }();

        template<class T>
        struct instance_for_null_t
        {
            [[nodiscard]] constexpr T operator () () const noexcept
            {
                if constexpr (std::is_void_v<T>)
                {
                    return;
                }
                else
                {
                    return T(null_v<T>);
                }
            }
        };

        template<class T>
        constexpr instance_for_null_t<T> instance_for_null{};
    }


    namespace private_detail_cmp_null
    {
        using private_detail_null_type::null_v;
        using private_detail_null_type::is_null_constructible;
        using namespace private_detail_zero::private_detail_compare;

        template<class T, template<class, class> class Op>
        using op_result_t = typename std::enable_if_t<std::conjunction_v<std::negation<is_null_type<T>>, is_null_constructible<T>>, enable_if_detected<Op, T, T>>::type;

        template<class T>
        using eq_op_result_t = op_result_t<T, decl_eq_op_t>;

        template<class T>
        using neq_op_result_t = op_result_t<T, decl_neq_op_t>;

        template<class T>
        [[nodiscard]] constexpr eq_op_result_t<T> eqn(const T& value) noexcept
        {
            return eq_op<T>(value, null_v<T>);
        }

        template<class T>
        [[nodiscard]] constexpr neq_op_result_t<T> neqn(const T& value) noexcept
        {
            return neq_op<T>(value, null_v<T>);
        }
    }


    namespace private_detail_has_value
    {
        using private_detail_cmp_null::eqn;
        using private_detail_cmp_null::neqn;
        using namespace ordered_overload;

        template<class T>
        [[nodiscard]] constexpr auto has_value_helper(const T& value, _order<_2>) noexcept
            -> decltype(!eqn(value))
        {
            return !eqn(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto has_value_helper(const T& value, _order<_1>) noexcept
            -> decltype(neqn(value))
        {
            return neqn(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto has_value_helper(const T& value, _order<_0>) noexcept
            -> decltype(!!value)
        {
            return !!value;
        }

        template<class T>
        [[nodiscard]] constexpr auto has_value(const T& value) noexcept
            -> decltype(has_value_helper(value, _start))
        {
            return has_value_helper(value, _start);
        }
    }


    namespace private_detail_is_null
    {
        using private_detail_cmp_null::eqn;
        using private_detail_cmp_null::neqn;
        using namespace ordered_overload;

        template<class T>
        [[nodiscard]] constexpr auto is_null_helper(const T& value, _order<_2>) noexcept
            -> decltype(!eqn(value))
        {
            return !neqn(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto is_null_helper(const T& value, _order<_1>) noexcept
            -> decltype(neqn(value))
        {
            return eqn(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto is_null_helper(const T& value, _order<_0>) noexcept
            -> decltype(!value)
        {
            return !value;
        }

        template<class T>
        [[nodiscard]] constexpr auto is_null(const T& value) noexcept
            -> decltype(is_null_helper(value, _start))
        {
            return is_null_helper(value, _start);
        }
    }


    namespace private_detail_nullable
    {
        template<class T>
        using decl_has_value_t = decltype(private_detail_has_value::has_value(std::declval<const T&>()));

        template<class T>
        using is_nullable = is_detected<decl_has_value_t, T>;

        template<class T>
        constexpr bool is_nullable_v = is_nullable<T>::value;
    }


    namespace private_detail_decl_null_eq_op
    {
        using private_detail_is_null::is_null;
        using private_detail_has_value::has_value;

        template<class T>
        [[nodiscard]] constexpr auto operator == (const T& value, decl_null_type_t<T>) noexcept -> decltype(is_null(value))
        {
            return is_null(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator == (decl_null_type_t<T>, const T& value) noexcept -> decltype(is_null(value))
        {
            return is_null(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator != (const T& value, decl_null_type_t<T>) noexcept -> decltype(has_value(value))
        {
            return has_value(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator != (decl_null_type_t<T>, const T& value) noexcept -> decltype(has_value(value))
        {
            return has_value(value);
        }
    }
}

using private_detail_null::private_detail_null_type::is_null_constructible;
using private_detail_null::private_detail_null_type::is_null_constructible_v;
using private_detail_null::private_detail_null_type::null_t;
using private_detail_null::private_detail_null_type::instance_null_t;
using private_detail_null::private_detail_null_type::instance_null;
using private_detail_null::private_detail_null_type::null_v;
using private_detail_null::private_detail_null_type::instance_for_null;
using private_detail_null::private_detail_has_value::has_value;
using private_detail_null::private_detail_is_null::is_null;
using private_detail_null::private_detail_nullable::is_nullable;
using private_detail_null::private_detail_nullable::is_nullable_v;
using private_detail_null::private_detail_decl_null_eq_op::operator==;
using private_detail_null::private_detail_decl_null_eq_op::operator!=;

