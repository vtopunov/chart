#pragma once

#include <type_traits>


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
    };

    template <class Default, template<class...> class Op, class... Args>
    struct detector<Default, std::void_t<Op<Args...>>, Op, Args...>
    {
        using value_t = std::true_type;
        using type = Op<Args...>;
    };
}

template <template<class...> class Op, class... Args>
using is_detected = typename private_detail_member_detector::detector<nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
using detected_t = typename private_detail_member_detector::detector<nonesuch, void, Op, Args...>::type;

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
struct add_const_pointer<T*const>
{
    using type = const T*const;
};

template<class T>
using add_const_pointer_t = typename add_const_pointer<T>::type;


template<class T> [[nodiscard]]
constexpr decltype(auto) as_unsigned(const T& value) noexcept
{
    return static_cast<std::make_unsigned_t<T>>(value);
}

template<class T> [[nodiscard]]
constexpr std::enable_if_t<std::is_arithmetic_v<T>, remove_unsigned_t<T>> as_signed(const T& value) noexcept
{
    return static_cast<remove_unsigned_t<T>>(value);
}