#pragma once

#include <type_traits>

#include <core/fwd.h>


static_assert(std::is_same_v<std::make_signed_t<size_t>, ptrdiff_t>);
static_assert(std::is_same_v<size_t, std::make_unsigned_t<ptrdiff_t>>);


namespace ordered_overload
{
    struct _3
    {};

    struct _2 : _3
    {};

    struct _1 : _2
    {};

    struct _0 : _1
    {};

    template<class T>
    using _overload = const T* const;

    template<class T>
    using _order = _overload<T>;

    template<class T>
    constexpr _overload<T> _overload_v{ nullptr };

    constexpr _order<_0> _start{ nullptr };
}

template<class T, class... Args>
using subapply_result_t = std::remove_cvref_t<decltype(std::declval<T&>().apply(std::declval<Args>()...))>;

template<class T>
using const_lvalue_reference_t = std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<T>>>;


namespace private_detail_type_traits
{
    namespace private_detail_member_detector
    {
        template <class Default, class AlwaysVoid,
            template<class...> class Op, class... Args>
        struct detector
        {
            using value_t = std::false_type;
            using type = Default;
            using enable_if_type = Default;
            using enable_if_and_type = dummy;
        };

        template <class Default, template<class...> class Op, class... Args>
        struct detector<Default, std::void_t<Op<Args...>>, Op, Args...>
        {
            using value_t = std::true_type;
            using type = Op<Args...>;
            using enable_if_type = std::enable_if<true, type>;
            using enable_if_and_type = std::enable_if<true, Default>;
        };
    }
}

template <class Default, template<class...> class Op, class... Args>
using detected_or = private_detail_type_traits::private_detail_member_detector::detector<Default, void, Op, Args...>;

template <template<class...> class Op, class... Args>
using is_detected = typename detected_or<dummy, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
using detected_t = typename detected_or<dummy, Op, Args...>::type;

template <class Default, template<class...> class Op, class... Args>
using enable_if_detected_or = typename detected_or<Default, Op, Args...>::enable_if_type;

template <class Result, template<class...> class Op, class... Args>
using enable_if_detected_and = typename detected_or<Result, Op, Args...>::enable_if_and_type;

template <template<class...> class Op, class... Args>
using enable_if_detected = enable_if_detected_or<dummy, Op, Args...>;

template< class Default, template<class...> class Op, class... Args >
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

template <template<class...> class Op, class... Args>
using enable_if_detected_t = typename enable_if_detected<Op, Args...>::type;

template <class Default, template<class...> class Op, class... Args>
using enable_if_detected_or_t = typename enable_if_detected_or<Default, Op, Args...>::type;

template <class Result, template<class...> class Op, class... Args>
using enable_if_detected_and_t = typename enable_if_detected_and<Result, Op, Args...>::type;

template <class Expected, template<class...> class Op, class... Args>
using is_detected_exact = std::is_same<Expected, detected_or_t<ttypes<Expected>, Op, Args...>>;

template <class To, template<class...> class Op, class... Args>
using is_detected_convertible = std::is_convertible<detected_or_t<ttypes<To>, Op, Args...>, To>;

template< template<class...> class Op, class... Args >
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template <class Expected, template<class...> class Op, class... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

template <class To, template<class...> class Op, class... Args>
constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;


template<size_t Value>
using index_constant = std::integral_constant<size_t, Value>;

template<size_t Value>
constexpr index_constant<Value> index_constant_v{};


struct less_fn
{
    template <class L, class R>
    [[nodiscard]] constexpr auto operator()(const L& left, const R& right) const noexcept
        -> decltype(left < right)
    {
        return left < right;
    }
};

constexpr less_fn less_op{};

template<class L, class R>
using is_less = std::bool_constant<!!less_op(L::value, R::value)>;

template<class L, class R>
constexpr bool is_less_v = is_less<L, R>::value;


template<class L, class... OrR>
using is_same_or = std::disjunction<std::is_same<L, OrR>...>;

template<class L, class... OrR>
constexpr bool is_same_or_v = is_same_or<L, OrR...>::value;


template<bool test, template<class...> class Op0, template<class...> class Op1, class... Args>
struct select_op
{
    using type = Op1<Args...>;
};

template<template<class...> class Op0, template<class...> class Op1, class... Args>
struct select_op<false, Op0, Op1, Args...>
{
    using type = Op0<Args...>;
};

template<bool test, class Default, template<class...> class Op, class... Args>
struct conditional_op_or
{
    using type = Op<Args...>;
};

template<class Default, template<class...> class Op, class... Args>
struct conditional_op_or<false, Default, Op, Args...>
{
    using type = Default;
};

template<bool test, template<class...> class Op, class... Args>
struct conditional_op;

template<bool test, template<class...> class Op, class Arg0, class... Args>
struct conditional_op<test, Op, Arg0, Args...> : conditional_op_or<test, Arg0, Op, Arg0, Args...>
{};

template<bool test, class Default, template<class...> class Op, class... Args>
using conditional_op_or_t = typename conditional_op_or<test, Default, Op, Args...>::type;

template<bool test, template<class...> class Op, class... Args>
using conditional_op_t = typename conditional_op<test, Op, Args...>::type;

template<bool test, template<class...> class Op0, template<class...> class Op1, class... Args>
using select_op_t = typename select_op<test, Op0, Op1, Args...>::type;


template<bool test, class T>
using conditional_add_const = conditional_op<test, std::add_const_t, T>;

template<bool test, class T>
using conditional_add_pointer = conditional_op<test, std::add_pointer_t, T>;

template<bool test, class T>
using conditional_add_const_t = typename conditional_add_const<test, T>::type;

template<bool test, class T>
using conditional_add_pointer_t = typename conditional_add_pointer<test, T>::type;


template<class S, class D>
using copy_const_type = conditional_add_const<std::is_const_v<S>, D>;

template<class S, class D>
using copy_pointer_type = conditional_add_pointer<std::is_pointer_v<S>, D>;

template<class S, class D>
using copy_signed_type = select_op<std::is_unsigned_v<S>, std::make_signed_t, std::make_unsigned_t, D>;

template<class S, class D>
using copy_const_t = typename copy_const_type<S, D>::type;

template<class S, class D>
using copy_pointer_t = typename copy_pointer_type<S, D>::type;

template<class S, class D>
using copy_signed_t = typename copy_signed_type<S, D>::type;


template<class Value, class Old, class New>
using replace_t = std::conditional_t<std::is_same_v<Value, Old>, New, Value>;

template<class Value, class New>
using replace_void_t = std::conditional_t<std::is_void_v<Value>, New, Value>;


template <class T>
using remove_enum_t = conditional_op_t<std::is_enum_v<T>, std::underlying_type_t, T>;

template<class T>
using remove_cve_t = std::remove_cv_t<remove_enum_t<T>>;

template<class T>
using remove_cveref_t = std::remove_cvref_t<remove_enum_t<T>>;

template <class T>
constexpr bool is_nonbool_integral_v = std::conjunction_v<
    std::is_integral<T>,
    std::negation<std::is_same<std::remove_cv_t<T>, bool> >
>;

template<class T>
using unsigned_or_t = conditional_op_t<::is_nonbool_integral_v<T>, std::make_unsigned_t, T>;

template<class T>
using enable_if_make_unsigned_t = typename conditional_op_or_t<::is_nonbool_integral_v<T>, dummy, std::make_unsigned, T>::type;

template<class T>
using remove_unsigned_t = conditional_op_t<std::is_unsigned_v<T>, std::make_signed_t, T>;

template<class T>
struct remove_noexcept
{
    using type = T;
};

template<class R, class... Args>
struct remove_noexcept<noexcept_function_pointer_t<R, Args...>>
{
    using type = function_pointer_t<R, Args...>;
};

template<class T>
using remove_noexcept_t = typename remove_noexcept<T>::type;


template<class T>
struct add_const_pointer;

template<class T>
struct add_const_pointer<T*>
{
    using type = const T*;
};

template<class T>
struct add_const_pointer<T* const>
{
    using type = const T* const;
};

template<class T>
using add_const_pointer_t = typename add_const_pointer<T>::type;

template<class T>
struct add_noexcept
{
    using type = T;
};

template<class R, class... Args>
struct add_noexcept<function_pointer_t<R, Args...>>
{
    using type = noexcept_function_pointer_t<R, Args...>;
};

template<class T>
using add_noexcept_t = typename add_noexcept<T>::type;

namespace private_detail_type_traits
{
    namespace private_detail_has_no_unique_address
    {
        template<class Align, class T>
        struct use_no_unique_address
        {
            Align padding;
            D_NO_UNIQUE_ADDRESS std::remove_reference_t<T> value;
        };

        template<class Align, class T>
        using test_use_no_unique_address = std::bool_constant<sizeof(use_no_unique_address<Align, T>) == sizeof(Align)>;

        template<class T>
        using has_no_unique_address_helper = std::conjunction<test_use_no_unique_address<void*, T>, test_use_no_unique_address<max_align_t, T> >;

        template<class T>
        using has_no_unique_address = conditional_op_or_t<std::is_class_v<T>, std::false_type, has_no_unique_address_helper, T>;

        template<class T>
        constexpr bool has_no_unique_address_v = has_no_unique_address<T>::value;
    }
}

using private_detail_type_traits::private_detail_has_no_unique_address::has_no_unique_address;
using private_detail_type_traits::private_detail_has_no_unique_address::has_no_unique_address_v;


namespace private_detail_type_traits
{
    namespace private_detail_is_address
    {
        template<class T>
        using is_address_helper = std::disjunction<
            std::is_function<T>,
            std::is_pointer<T>,
            std::is_member_pointer<T>,
            std::is_null_pointer<T>
        >;
    }
}

template<class T>
using is_address = private_detail_type_traits::private_detail_is_address::is_address_helper<std::remove_reference_t<T>>;

template<class T>
constexpr bool is_address_v = is_address<T>::value;

template<class T>
using has_qualifier = std::disjunction<
    std::is_const<T>,
    std::is_volatile<T>,
    std::is_reference<T>,
    is_address<T>,
    std::is_array<T>
>;

template<class T>
constexpr bool has_qualifier_v = has_qualifier<T>::value;

template<class T>
using is_unqualified_class = std::conjunction<
    std::is_class<T>,
    std::negation<std::is_const<T>>,
    std::negation<std::is_volatile<T>>
>;

template<class T>
constexpr bool is_unqualified_class_v = is_unqualified_class<T>::value;


template<class From, class To>
struct is_const_convertible : std::false_type
{};

template<class T>
struct is_const_convertible<T, T> : std::true_type
{};

template<class T>
struct is_const_convertible<T, const T> : std::true_type
{};

template<class From, class To>
constexpr bool is_const_convertible_v = is_const_convertible<From, To>::value;


template<class T>
using add_sizeof_t = index_constant<sizeof(T)>;

template<class T>
using tr_sizeof = add_sizeof_t<replace_void_t<T, std::byte> >;

template<class T>
constexpr size_t sizeof_v = tr_sizeof<T>::value;


template<class T, T L, T R>
using is_same_int = std::bool_constant<L == R>;

template<class T, T L, T R>
using is_nsame_int = std::bool_constant<L != R>;

template<class T, T L, T R>
using is_less_int = std::bool_constant<less_op(L, R)>;

template<class T, T L, T R>
using is_less_equal_int = std::bool_constant<!less_op(R, L)>;

template<class T, T L, T R>
using is_greater_int = std::bool_constant<less_op(R, L)>;

template<class T, T L, T R>
using is_greater_equal_int = std::bool_constant<!less_op(L, R)>;


template<size_t L, size_t R>
using is_same_size = is_same_int<size_t, L, R>;

template<size_t L, size_t R>
using is_nsame_size = is_nsame_int<size_t, L, R>;

template<size_t L, size_t R>
using is_less_size = is_less_int<size_t, L, R>;

template<size_t L, size_t R>
using is_less_equal_size = is_less_equal_int<size_t, L, R>;

template<size_t L, size_t R>
using is_greater_size = is_greater_int<size_t, L, R>;

template<size_t L, size_t R>
using is_greater_equal_size = is_greater_equal_int<size_t, L, R>;


template<class L, class R>
using is_same_is_const = std::is_same<std::is_const<L>, std::is_const<R>>;

template<class L, class R>
using is_same_uncvref_r = std::is_same<L, std::remove_cvref_t<R>>;

template<class L, class R>
using is_same_uncvref = is_same_uncvref_r<std::remove_cvref_t<L>, R>;

template<class L, class R>
using is_same_uncv_r = std::is_same<L, std::remove_cv_t<R>>;

template<class L, class R>
using is_same_uncv = is_same_uncv_r<std::remove_cv_t<L>, R>;

template<class L, class R>
using is_same_decay_r = std::is_same<L, std::decay_t<R>>;

template<class L, class R>
using is_same_decay = is_same_decay_r<std::decay_t<L>, R>;

template<class L, class R>
constexpr auto is_same_uncvref_v = is_same_uncvref<L, R>::value;

template<class L, class R>
constexpr auto is_same_uncv_v = is_same_uncv<L, R>::value;

template<class L, class R>
constexpr auto is_same_decay_v = is_same_decay<L, R>::value;


template <class T, class... Types>
using has_type = std::disjunction<std::is_same<T, Types>...>;

template <class T, class... Types>
using has_type_uncvref = std::disjunction<is_same_uncvref_r<T, Types>...>;

template <class T, class... Types>
using has_type_decay = std::disjunction<is_same_decay_r<T, Types>...>;


template <class T, class... Types>
constexpr bool has_type_v = has_type<T, Types...>::value;

template <class T, class... Types>
constexpr bool has_type_uncvref_v = has_type_uncvref<T, Types...>::value;

template <class T, class... Types>
constexpr bool has_type_decay_v = has_type_decay<T, Types...>::value;


template<class T>
[[nodiscard]] constexpr enable_if_make_unsigned_t<T> as_unsigned(const T& value) noexcept
{
    return value;
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<T>, remove_unsigned_t<T>> as_signed(const T& value) noexcept
{
    return static_cast<remove_unsigned_t<T>>(value);
}

template<class Derived, class Base>
struct identical_derived
{
    constexpr explicit operator bool() const noexcept
    {
        using derived_t = std::remove_cvref_t<Derived>;
        constexpr bool b_is_unqualified_class = is_unqualified_class_v<Base>;
        constexpr bool d_is_unqualified_class = is_unqualified_class_v<derived_t>;
        constexpr bool is_base_of = std::is_base_of_v<Base, derived_t>;
        constexpr bool eq_sizeof = sizeof(Base) == sizeof(derived_t);
        constexpr bool eq_alignof = alignof(Base) == alignof(derived_t);

        return b_is_unqualified_class
            && d_is_unqualified_class
            && is_base_of
            && eq_sizeof
            && eq_alignof;
    }
};

template<class Derived, class Base>
constexpr identical_derived<Derived, Base> identical_derived_v{};

template<class Derived, class Base>
[[nodiscard]] constexpr const Derived& to_identical_derived(const Base& base) noexcept
{
    static_assert(identical_derived_v<Derived, Base>);
    return static_cast<const Derived&>(base);
}


template<class T>
constexpr bool is_pointer_or_nullptr_v = std::disjunction_v<std::is_pointer<T>, std::is_null_pointer<T>>;


template<class T, class... Args>
using decl_brace_construct_t = decltype(new (std::declval<T*>()) T{ std::declval<Args>()... });

template<class T, class... Args>
using is_brace_constructible = is_detected<decl_brace_construct_t, T, Args...>;

template<class T, class... Args>
constexpr bool is_brace_constructible_v = is_brace_constructible<T, Args...>::value;


template<class T>
using decl_unary_munis_result_t = decltype(-std::declval<const T&>());

template<class T>
using decl_pre_inc_op_t = decltype(++std::declval<T&>());

template<class T>
using decl_post_inc_op_t = decltype(std::declval<T&>()++);

template<class T>
using decl_pre_dec_op_t = decltype(--std::declval<T&>());

template<class T>
using decl_post_dec_op_t = decltype(std::declval<T&>()--);

template<class T>
using has_unary_munis_op = is_detected<decl_unary_munis_result_t, T>;

template<class T>
using has_pre_inc_op = is_detected<decl_pre_inc_op_t, T>;

template<class T>
using has_post_inc_op = is_detected<decl_post_inc_op_t, T>;

template<class T>
using has_pre_dec_op = is_detected<decl_pre_dec_op_t, T>;

template<class T>
using has_post_dec_op = is_detected<decl_post_dec_op_t, T>;

template<class T>
constexpr bool has_unary_munis_op_v = has_unary_munis_op<T>::value;

template<class T>
constexpr bool has_pre_inc_op_v = has_pre_inc_op<T>::value;

template<class T>
constexpr bool has_post_inc_op_v = has_post_inc_op<T>::value;

template<class T>
constexpr bool has_pre_dec_op_v = has_pre_dec_op<T>::value;

template<class T>
constexpr bool has_post_dec_op_v = has_post_dec_op<T>::value;


template<class L, class R>
using decl_assignment_op_t = decltype(std::declval<L&>() = std::declval<R>());

template<class L, class R>
using has_assignment_op = is_detected<decl_assignment_op_t, L, R>;

template<class L, class R>
constexpr bool has_assignment_op_v = has_assignment_op<L, R>::value;

template<class L, class R = L>
using decl_eq_op_t = decltype(std::declval<const L&>() == std::declval<const R&>());

template<class L, class R = L>
using decl_neq_op_t = decltype(std::declval<const L&>() != std::declval<const R&>());

template<class L, class R = L>
using decl_n_op_eq_op_t = decltype(!(std::declval<const L&>() == std::declval<const R&>()));

template<class L, class R = L>
using decl_n_op_neq_op_t = decltype(!(std::declval<const L&>() != std::declval<const R&>()));

template<class L, class R = L>
using decl_less_op_t = decltype(less_op(std::declval<const L&>(), std::declval<const R&>()));

template<class L, class R = L>
using decl_nless_op_t = decltype(!std::declval<decl_less_op_t<L, R>>());


template<class T>
[[nodiscard]] constexpr std::enable_if_t<has_pre_inc_op_v<T>, T> u_next(T value) noexcept
{
    return ++value;
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<has_pre_dec_op_v<T>, T> u_prev(T value) noexcept
{
    return --value;
}
