#pragma once

#include <tuple>

#include <core/type_traits.h>


template<template <class> class Fn, class Tuple>
struct transform_tuple_type;

template<template <class> class Fn, typename... Types>
struct transform_tuple_type<Fn, std::tuple<Types...>>
{
    using type = std::tuple<Fn<Types>...>;
};

template<template <class> class Fn, class Tuple>
using transform_tuple_t = typename transform_tuple_type<Fn, Tuple>::type;


template<class Tuple>
using make_tuple_index_sequence = std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>;


template <class T, class Tuple>
struct tuple_has_type;

template <class T, class... Types>
struct tuple_has_type<T, std::tuple<Types...>> : std::disjunction<std::is_same<T, Types>...> {};

template <class T, class Tuple>
constexpr auto tuple_has_type_v = tuple_has_type<T, Tuple>::value;


template <class Tuple, class... Args>
struct tuple_has_call;

template <class... Types, class... Args>
struct tuple_has_call<std::tuple<Types...>, Args...> : std::disjunction<call_is_detected<Types, Args...>...> {};

template <class Tuple, class... Args>
constexpr auto tuple_has_call_v = tuple_has_call<Tuple, Args...>::value;


template <size_t I, size_t J, class IS>
struct swap_index_sequence;

template <size_t I, size_t J, size_t... Indices>
struct swap_index_sequence<I, J, std::index_sequence<Indices...>>
{
    using type = std::index_sequence<((Indices != I) ? ((Indices != J) ? Indices : I) : J)...>;
};

template <size_t I, size_t J, class IS>
using swap_index_sequence_t = typename swap_index_sequence<I, J, IS>::type;


template<class IS>
struct index_sequence_pop_front;

template <size_t I, size_t... Indices>
struct index_sequence_pop_front<std::index_sequence<I, Indices...> >
{
    using type = std::index_sequence<Indices...>;
};

template <class IS>
using index_sequence_pop_front_t = typename index_sequence_pop_front<IS>::type;


template<size_t I0, class IS>
struct index_sequence_push_front;

template <size_t I0, size_t... Indices>
struct index_sequence_push_front<I0, std::index_sequence<Indices...> >
{
    using type = std::index_sequence<I0, Indices...>;
};

template <size_t I0, class IS>
using index_sequence_push_front_t = typename index_sequence_push_front<I0, IS>::type;


template<class Tuple, class Is>
struct reorder_tuple_type;

template<class Tuple, size_t... Indices>
struct reorder_tuple_type<Tuple, std::index_sequence<Indices...> >
{
    using type = std::tuple<std::tuple_element_t<Indices, Tuple>...>;
};

template<class Tuple, class Is>
using reorder_tuple_t = typename reorder_tuple_type<Tuple, Is>::type;


template <size_t I, size_t J, class Tuple>
using tuple_swap_type = reorder_tuple_type<
    Tuple,
    swap_index_sequence_t<I, J, make_tuple_index_sequence<Tuple>>
>;

template<size_t I, size_t J, class Tuple>
using tuple_swap_t = typename tuple_swap_type<I, J, Tuple>::type;


template <class Tuple>
struct tuple_pop_front_type;

template <class T, class... Types>
struct tuple_pop_front_type<std::tuple<T, Types...>>
{
    using type = std::tuple<Types...>;
};

template <class Tuple>
using tuple_pop_front_t = typename tuple_pop_front_type<Tuple>::type;


template <class T, class Tuple>
struct tuple_push_front_type;

template <class T, class... Types>
struct tuple_push_front_type<T, std::tuple<Types...>>
{
    using type = std::tuple<T, Types...>;
};

template <class T, class Tuple>
using tuple_push_front_t = typename tuple_push_front_type<T, Tuple>::type;


template <class Tuple, class T>
struct tuple_push_back_type;

template <class... Types, class T>
struct tuple_push_back_type<std::tuple<Types...>, T>
{
    using type = std::tuple<Types..., T>;
};

template <class Tuple, class T>
using tuple_push_back_t = typename tuple_push_back_type<Tuple, T>::type;

template<class Tuple, bool test, class T>
using tuple_push_back_if_t = conditional_op_or_t<test, Tuple, tuple_push_back_t, Tuple, T>;

template <class... Tuples>
struct tuple_cat_type;

template <class... Types>
struct tuple_cat_type<std::tuple<Types...>>
{
    using type = std::tuple<Types...>;
};

template <class... Types0, class... Types1, class... Tuples>
struct tuple_cat_type<std::tuple<Types0...>, std::tuple<Types1...>, Tuples...>
{
    using type = typename tuple_cat_type<std::tuple<Types0..., Types1...>, Tuples...>::type;
};

template <class... Tuples>
using tuple_cat_t = typename tuple_cat_type<Tuples...>::type;


template<template <class, class> class Cmp, class Tuple, class IS>
struct min_tuple_index_element_type;

template<template <class, class> class Cmp, class Tuple, size_t I0>
struct min_tuple_index_element_type<Cmp, Tuple, std::index_sequence<I0> > : std::integral_constant<size_t, I0>
{};

template<template <class, class> class Cmp, class Tuple, size_t I0, size_t I1, size_t... Indices>
struct min_tuple_index_element_type<Cmp, Tuple, std::index_sequence<I0, I1, Indices...> > : std::conditional_t<
    Cmp<
    std::tuple_element_t<I0, Tuple>,
    std::tuple_element_t<I1, Tuple>
    >::value,
    min_tuple_index_element_type<Cmp, Tuple, std::index_sequence<I0, Indices...> >,
    min_tuple_index_element_type<Cmp, Tuple, std::index_sequence<I1, Indices...> >
>
{};

template<template <class, class> class Cmp, class Tuple, class IS = make_tuple_index_sequence<Tuple> >
constexpr auto min_tuple_index_element_v = min_tuple_index_element_type<Cmp, Tuple, IS>::value;


template<template <class, class> class Cmp, class Tuple, class IS>
struct sort_tuple_indices;

template<template <class, class> class Cmp, class Tuple>
struct sort_tuple_indices<Cmp, Tuple, std::index_sequence<> >
{
    using type = std::index_sequence<>;
};

template<template <class, class> class Cmp, class Tuple, size_t I0, size_t... Indices>
struct sort_tuple_indices<Cmp, Tuple, std::index_sequence<I0, Indices...> >
{
    using index_sequence_type = std::index_sequence<I0, Indices...>;
    static constexpr auto min_index_element_value = min_tuple_index_element_v<Cmp, Tuple, index_sequence_type>;

    using tail = index_sequence_pop_front_t<swap_index_sequence_t<I0, min_index_element_value, index_sequence_type>>;
    using sorted_tail = typename sort_tuple_indices<Cmp, Tuple, tail>::type;

    using type = index_sequence_push_front_t<
        min_index_element_value,
        sorted_tail
    >;
};

template<template <class, class> class Cmp, class Tuple, class IS = make_tuple_index_sequence<Tuple>>
using sort_tuple_indices_t = typename sort_tuple_indices<Cmp, Tuple, IS>::type;

template <template <class, class> class Cmp, class Tuple>
using sort_tuple_type = reorder_tuple_type<Tuple, sort_tuple_indices_t<Cmp, Tuple>>;

template <template <class, class> class Cmp, class Tuple>
using sort_tuple_t = typename sort_tuple_type<Cmp, Tuple>::type;


template <class T, class U>
constexpr bool less_sizeof_v = (sizeof(T) < sizeof(U));

template <class T, class U>
constexpr bool greater_sizeof_v = (sizeof(T) > sizeof(U));

template <class T, class U>
constexpr bool less_eq_sizeof_v = (sizeof(T) <= sizeof(U));

template <class T, class U>
constexpr bool greater_eq_sizeof_v = (sizeof(T) >= sizeof(U));

template <class T, class U>
using less_sizeof = std::bool_constant<less_sizeof_v<T, U>>;

template <class T, class U>
using greater_sizeof = std::bool_constant<greater_sizeof_v<T, U>>;

template <class T, class U>
using less_eq_sizeof = std::bool_constant<less_eq_sizeof_v<T, U>>;

template <class T, class U>
using greater_eq_sizeof = std::bool_constant<greater_eq_sizeof_v<T, U>>;


template <class Tuple>
using tuple_sizeof_optimization_type = sort_tuple_type<greater_eq_sizeof, Tuple>;

template<class Tuple>
using tuple_sizeof_optimization_t = typename tuple_sizeof_optimization_type<Tuple>::type;


template<class Tuple, class T>
using tuple_unique_push_back_t = tuple_push_back_if_t<Tuple, std::negation_v<tuple_has_type<T, Tuple>>, T>;

template<class Tuple, class... Types>
struct tuple_unique_insert_back_type;

template<class Tuple>
struct tuple_unique_insert_back_type<Tuple>
{
    using type = Tuple;
};

template<class Tuple, class T0, class... Types>
struct tuple_unique_insert_back_type<Tuple, T0, Types...>
{
    using type = typename tuple_unique_insert_back_type<
        tuple_unique_push_back_t<Tuple, T0>,
        Types...
    >::type;
};

template<class Tuple, class... Types>
using tuple_unique_insert_back_t = typename tuple_unique_insert_back_type<Tuple, Types...>::type;

template<class Tuple, class TailTuple>
struct tuple_unique_push_back_tuple_type;

template<class Tuple, class... Types>
struct tuple_unique_push_back_tuple_type<Tuple, std::tuple<Types...> >
{
    using type = tuple_unique_insert_back_t<Tuple, Types...>;
};

template<class Tuple, class TailTuple>
using tuple_unique_push_back_tuple_t = typename tuple_unique_push_back_tuple_type<Tuple, TailTuple>::type;

template<class Tuple>
using unique_tuple_t = tuple_unique_push_back_tuple_t<std::tuple<>, Tuple>;

template<class Tuple, class... Types>
struct tuple_unique_insert_back_tuple_type;

template<class Tuple>
struct tuple_unique_insert_back_tuple_type<Tuple>
{
    using type = Tuple;
};

template<class Tuple, class T0, class... Types>
struct tuple_unique_insert_back_tuple_type<Tuple, T0, Types...>
{
    using type = typename tuple_unique_insert_back_tuple_type<
        tuple_unique_push_back_tuple_t<Tuple, T0>,
        Types...
    >::type;
};

template<class Tuple, class... Types>
using tuple_unique_insert_back_tuple_t = typename tuple_unique_insert_back_tuple_type<Tuple, Types...>::type;