#pragma once

#include <type_traits>

#include <core/fwd.h>


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

struct no_overload
{
    template<class T>
    constexpr no_overload(const T&) noexcept
    {}
};

template<class T>
struct no_overload_for
{
    constexpr no_overload_for(const T&) noexcept
    {}
};

template<class Fn, class... Args>
constexpr auto call_if_exist(Fn&& fn, Args&&... args) noexcept -> decltype(std::forward<Fn>(fn)(std::forward<Args>(args)...))
{
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
}

template <class... Args>
constexpr auto call_if_exist(no_overload, const Args&... args) noexcept -> std::void_t<decltype(no_overload(args))...>
{}


struct nonesuch
{
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
};

namespace private_detail_member_detector
{
    template <class Default, class AlwaysVoid,
        template<class...> class Op, class... Args>
    struct detector
    {
        using value_t = std::false_type;
        using type = Default;
        using enable_if_type = type;
    };

    template <class Default, template<class...> class Op, class... Args>
    struct detector<Default, std::void_t<Op<Args...>>, Op, Args...>
    {
        using value_t = std::true_type;
        using type = Op<Args...>;
        using enable_if_type = std::type_identity<type>;
    };
}

template <template<class...> class Op, class... Args>
using is_detected = typename private_detail_member_detector::detector<nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
using detected_t = typename private_detail_member_detector::detector<nonesuch, void, Op, Args...>::type;

template <template<class...> class Op, class... Args>
using enable_if_detected = typename private_detail_member_detector::detector<nonesuch, void, Op, Args...>::enable_if_type;

template <class Default, template<class...> class Op, class... Args>
using detected_or = private_detail_member_detector::detector<Default, void, Op, Args...>;

template< template<class...> class Op, class... Args >
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template< class Default, template<class...> class Op, class... Args >
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

template <class Expected, template<class...> class Op, class... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

template <class Expected, template<class...> class Op, class... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

template <class To, template<class...> class Op, class... Args>
using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;

template <class To, template<class...> class Op, class... Args>
constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;


template<class Fn, class... Args>
using decl_call_t = decltype(std::declval<Fn>()(std::declval<Args>()...));

template<class Fn, class... Args>
using call_is_detected = is_detected<decl_call_t, Fn, Args...>;

template<class Fn, class... Args>
constexpr bool call_is_detected_v = call_is_detected<Fn, Args...>::value;


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
using copy_const_t = typename copy_const_type<S, D>::type;

template<class S, class D>
using copy_pointer_t = typename copy_pointer_type<S, D>::type;


template<class Value, class Old, class New>
using replace_type = std::conditional<std::is_same_v<Value, Old>, New, Value>;

template<class Value, class Old, class New>
using replace_t = typename replace_type<Value, Old, New>::type;


template <class T>
using remove_enum = conditional_op<std::is_enum_v<T>, std::underlying_type_t, T>;

template <class T>
using remove_enum_t = typename remove_enum<T>::type;

template<class T>
using remove_cve_t = std::remove_cv_t<remove_enum_t<T>>;

template<class T>
using remove_cveref_t = std::remove_cvref_t<remove_enum_t<T>>;


template<class T>
using unsigned_or_t = conditional_op_t<std::is_integral_v<T>, std::make_unsigned_t, T>;

template<class T>
using remove_unsigned_t = conditional_op_t<std::is_unsigned_v<T>, std::make_signed_t, T>;

template<class T>
struct add_const_pointer {};

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


template<class L, class R>
using is_same_uncvref = std::is_same<std::remove_cvref_t<L>, std::remove_cvref_t<R>>;

template<class L, class R>
constexpr auto is_same_uncvref_v = is_same_uncvref<L, R>::value;


template<class T>
[[nodiscard]] constexpr decltype(auto) as_unsigned(const T& value) noexcept
{
    return static_cast<std::make_unsigned_t<T>>(value);
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<T>, remove_unsigned_t<T>> as_signed(const T& value) noexcept
{
    return static_cast<remove_unsigned_t<T>>(value);
}

template<class Derived, class Base>
[[nodiscard]] constexpr Derived identical_derived_cast(const Base& base) noexcept
{
    {
        using derived_t = std::remove_reference_t<Derived>;
        static_assert(std::is_base_of_v<Base, derived_t>);
        static_assert(sizeof(Base) == sizeof(derived_t));
    }
    return static_cast<Derived>(base);
}


template<class T>
constexpr bool is_pointer_or_nullptr_v = std::disjunction_v<std::is_pointer<T>, std::is_null_pointer<T>>;

template<class T>
using decl_unary_munis_op_t = decltype(-std::declval<const T&>());

template<class T>
using decl_pre_inc_op_t = decltype(++std::declval<T&>());

template<class T>
using decl_post_inc_op_t = decltype(std::declval<T&>()++);

template<class T>
using decl_pre_dec_op_t = decltype(--std::declval<T&>());

template<class T>
using decl_post_dec_op_t = decltype(std::declval<T&>()--);

template<class T>
using has_unary_munis_op = is_detected<decl_unary_munis_op_t, T>;

template<class T>
using has_pre_inc_op = is_detected<decl_pre_inc_op_t, T>;

template<class T>
using has_post_inc_op = is_detected<decl_post_inc_op_t, T>;

template<class T>
using has_pre_dec_op = is_detected<decl_pre_dec_op_t, T>;

template<class T>
using has_post_dec_op = is_detected<decl_post_dec_op_t, T>;


template<class L, class R>
using decl_assignment_op_t = decltype(std::declval<L&>() = std::declval<R>());

template<class L, class R>
using has_assignment_op = is_detected<decl_assignment_op_t, L, R>;

template<class L, class R>
constexpr auto has_assignment_op_v = has_assignment_op<L, R>::value;


namespace type_traits_compare
{
    template<class T>
    [[nodiscard]] constexpr auto eq_op(const T& left, const T& right) noexcept -> decltype(left == right)
    {
        return left == right;
    }

    template<class T>
    [[nodiscard]] constexpr auto neq_op(const T& left, const T& right) noexcept -> decltype(left != right)
    {
        return left != right;
    }

    template<class T>
    [[nodiscard]] constexpr auto less_op(const T& left, const T& right) noexcept -> decltype(left < right)
    {
        return left < right;
    }

    template<class T>
    using decl_eq_op_t = decltype(eq_op(std::declval<const T&>(), std::declval<const T&>()));

    template<class T>
    using decl_neq_op_t = decltype(neq_op(std::declval<const T&>(), std::declval<const T&>()));

    template<class T>
    using decl_less_op_t = decltype(less_op(std::declval<const T&>(), std::declval<const T&>()));
}

using type_traits_compare::decl_eq_op_t;
using type_traits_compare::decl_neq_op_t;
using type_traits_compare::decl_less_op_t;

template<class T>
using has_eq_op = is_detected<decl_eq_op_t, T>;

template<class T>
using has_neq_op = is_detected<decl_neq_op_t, T>;

template<class T>
using has_less_op = is_detected<decl_less_op_t, T>;