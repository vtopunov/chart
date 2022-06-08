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
