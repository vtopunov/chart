#pragma once

#include <type_traits>


template<bool test, template<class> class Op, class T>
struct conditional_op 
{ 
    using type = Op<T>;
};

template<template<class> class Op, class T>
struct conditional_op<false, Op, T> 
{
    using type = T;
};

template<bool test, template<class> class Op, class T>
using conditional_op_t = typename conditional_op<test, Op, T>::type;


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
constexpr decltype(auto) as_unsigned_or(const T& value) noexcept
{
    static_assert(std::is_arithmetic_v<T>);
    return static_cast<unsigned_or_t<T>>(value);
}

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