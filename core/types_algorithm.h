#pragma once

#include <core/utility.h>


template<class T>
struct types_size : index_constant<0u> {};

template<template <class...> class Tuple, typename... Types>
struct types_size<Tuple<Types...>> : index_constant<sizeof...(Types)> {};

template<class T>
constexpr size_t types_size_v = types_size<T>::value;


template<class T>
struct types_sequence_size : index_constant<0u> {};

template<class SeqT, template <class, SeqT...> class Tuple, SeqT... Values>
struct types_sequence_size<Tuple<SeqT, Values...>> : index_constant<sizeof...(Values)> {};

template<class T>
constexpr size_t types_sequence_size_v = types_sequence_size<T>::value;


template<template <class> class Fn, class Tuple>
struct transform_types
{
    using type = Tuple;
};

template<template <class> class Fn, template <class...> class Tuple, typename... Types>
struct transform_types<Fn, Tuple<Types...>>
{
    using type = Tuple<Fn<Types>...>;
};

template<template <class> class Fn, class Tuple>
using transform_types_t = typename transform_types<Fn, Tuple>::type;

template<class Tuple>
using make_types_index_sequence = std::make_index_sequence<types_size_v<Tuple>>;


template <class T, class Tuple>
struct types_has_type : std::false_type
{};

template <class T, template <class...> class Tuple, typename... Types>
struct types_has_type<T, Tuple<Types...>> : has_type<T, Types...> {};

template <class T, class Tuple>
constexpr auto types_has_type_v = types_has_type<T, Tuple>::value;


template <class Tuple, class... Args>
struct types_has_call;

template <template <class...> class Tuple, class... Types, class... Args>
struct types_has_call<Tuple<Types...>, Args...> : std::disjunction<call_is_detected<Types, Args...>...> {};

template <class Tuple, class... Args>
constexpr auto types_has_call_v = types_has_call<Tuple, Args...>::value;


template <size_t I, size_t J, class IS>
struct swap_index_sequence;

template <size_t I, size_t J, size_t... Indices>
struct swap_index_sequence<I, J, std::index_sequence<Indices...>>
{
    using type = std::index_sequence<((Indices != I) ? ((Indices != J) ? Indices : I) : J)...>;
};

template <size_t I, size_t J, class IS>
using swap_index_sequence_t = typename swap_index_sequence<I, J, IS>::type;


template<size_t I0, class IS>
struct index_sequence_push_front;

template <size_t I0, size_t... Indices>
struct index_sequence_push_front<I0, std::index_sequence<Indices...> >
{
    using type = std::index_sequence<I0, Indices...>;
};

template <size_t I0, class IS>
using index_sequence_push_front_t = typename index_sequence_push_front<I0, IS>::type;


namespace private_detail_types_element
{
    template<size_t Index, class T>
    struct types_type_element
    {
        static T decl(index_constant<Index>);
    };

    template<class... Types>
    struct type_map : Types...
    {
        using Types::decl...;
    };

    template <class IS, class... Types>
    struct make_type_map_helper;

    template <size_t... Indices, class... Types>
    struct make_type_map_helper<std::index_sequence<Indices...>, Types...>
    {
        using type = type_map<types_type_element<Indices, Types>...>;
    };

    template<class... Types>
    using make_types_map_t = typename make_type_map_helper<std::make_index_sequence<sizeof...(Types)>, Types...>::type;

    template<size_t Index, class Map>
    using types_map_element_t = decltype(Map::decl(index_constant_v<Index>));

    template <size_t Index, class... Types>
    using types_element_t = types_map_element_t<Index, make_types_map_t<Types...>>;
}

using private_detail_types_element::make_types_map_t;
using private_detail_types_element::types_map_element_t;
using private_detail_types_element::types_element_t;


template<class, class Is>
struct reorder_types;

template<class Tuple, class Is>
struct reorder_types;

template<template <class...> class Tuple, class... Types, size_t... Indices>
struct reorder_types<Tuple<Types...>, std::index_sequence<Indices...> >
{
    using types_map_type = make_types_map_t<Types...>;

    using type = Tuple<types_map_element_t<Indices, types_map_type>...>;
};

template<class SeqT, template <class, SeqT...> class Tuple, SeqT... Values, size_t... Indices>
struct reorder_types<Tuple<SeqT, Values...>, std::index_sequence<Indices...> >
{
    static constexpr SeqT values_map[]{ Values... };

    using type = Tuple<SeqT, values_map[Indices]...>;
};

template<class Tuple, class Is>
using reorder_types_t = typename reorder_types<Tuple, Is>::type;


template <size_t I, size_t J, class Tuple>
using types_swap_type = reorder_types<
    Tuple,
    swap_index_sequence_t<I, J, make_types_index_sequence<Tuple>>
>;

template<size_t I, size_t J, class Tuple>
using types_swap_t = typename types_swap_type<I, J, Tuple>::type;


template <class Tuple>
struct types_pop_front;

template <template <class...> class Tuple, class T, class... Types>
struct types_pop_front<Tuple<T, Types...>>
{
    using type = Tuple<Types...>;
};

template <class SeqT, template <class, SeqT...> class Tuple, SeqT Front, SeqT... Values>
struct types_pop_front<Tuple<SeqT, Front, Values...>>
{
    using type = Tuple<SeqT, Values...>;
    static constexpr std::pair<type, SeqT> value{ {}, Front };
};

template <class Tuple>
using types_pop_front_t = typename types_pop_front<Tuple>::type;


template <class Tuple>
struct types_pop_back;

template <template <class...> class Tuple, class... Types>
struct types_pop_back<Tuple<Types...>>
{
    static constexpr size_t size{ sizeof...(Types) };
    static_assert(size > 0u);

    using type = reorder_types_t<Tuple<Types...>, std::make_index_sequence<size - 1u>>;
};

template <class SeqT, template <class, SeqT...> class Tuple, SeqT... Values>
struct types_pop_back<Tuple<SeqT, Values...>>
{
    static constexpr size_t size{ sizeof...(Values) };
    static constexpr size_t new_size{ size - 1u };
    using reorder_type = reorder_types<Tuple<SeqT, Values...>, std::make_index_sequence<new_size> >;

    using type = typename reorder_type::type;
    static constexpr std::pair<type, SeqT> value{ {}, reorder_type::values_map[new_size] };
};

template <class Tuple>
using types_pop_back_t = typename types_pop_back<Tuple>::type;


template <class T, class Tuple>
struct types_push_front_type;

template <class T, template <class...> class Tuple, class... Types>
struct types_push_front_type<T, Tuple<Types...>>
{
    using type = Tuple<T, Types...>;
};

template <class T, class Tuple>
using types_push_front_t = typename types_push_front_type<T, Tuple>::type;


template <class Tuple, class T>
struct types_push_back_type;

template <template <class...> class Tuple, class... Types, class T>
struct types_push_back_type<Tuple<Types...>, T>
{
    using type = Tuple<Types..., T>;
};

template <class Tuple, class T>
using types_push_back_t = typename types_push_back_type<Tuple, T>::type;

template<class Tuple, bool test, class T>
using types_push_back_if_t = conditional_op_or_t<test, Tuple, types_push_back_t, Tuple, T>;


template <template <class> class Pred, class Tuple, class Is, class ResultIs = std::index_sequence<> >
struct types_sequence_if;

template <template <class> class Pred, class Tuple, size_t... ResultIndices>
struct types_sequence_if<
    Pred, Tuple, std::index_sequence<>, std::index_sequence<ResultIndices...>
>
{
    using type = std::index_sequence<ResultIndices...>;
};

template<
    template <class> class Pred,
    template <class...> class Tuple,
    class... Types,
    size_t I0, size_t... Indices,
    size_t... ResultIndices
>
struct types_sequence_if<
    Pred, Tuple<Types...>,
    std::index_sequence<I0, Indices...>,
    std::index_sequence<ResultIndices...>
>
{
    using result_seq_type = std::conditional_t<
        Pred<types_element_t<I0, Types...>>::value,
        std::index_sequence<ResultIndices..., I0>,
        std::index_sequence<ResultIndices...>
    >;

    using type = typename types_sequence_if<Pred, Tuple<Types...>, std::index_sequence<Indices...>, result_seq_type>::type;
};

template<template <class> class Pred, class Tuple, class Is>
using types_sequence_if_t = typename types_sequence_if<Pred, Tuple, Is>::type;

template<template <class> class Pred, class Tuple>
using types_if_t = reorder_types_t<Tuple,
    types_sequence_if_t<Pred, Tuple, make_types_index_sequence<Tuple>>
>;


template <template <class> class Pred, class Tuple, class SourceSeq, class TreatSeq = std::index_sequence<> >
struct types_sequence_split_if;

template <template <class> class Pred, class Tuple, size_t... TreatIndices>
struct types_sequence_split_if<
    Pred, Tuple, std::index_sequence<>, std::index_sequence<TreatIndices...>
>
{
    using type = std::pair<std::index_sequence<>, std::index_sequence<>>;
};

template<
    template <class> class Pred,
    template <class...> class Tuple,
    class... Types,
    size_t I0, size_t... SourceIndices,
    size_t... TreatIndices
>
struct types_sequence_split_if<
    Pred, Tuple<Types...>,
    std::index_sequence<I0, SourceIndices...>,
    std::index_sequence<TreatIndices...>
>
{
    using source_seq = std::index_sequence<SourceIndices...>;
    using treat_seq = std::index_sequence<TreatIndices..., I0>;
    static constexpr bool has_split{ Pred<types_element_t<I0, Types...>>::value };

    using type = typename std::conditional_t<has_split,
        std::type_identity<std::pair<treat_seq, source_seq>>,
        types_sequence_split_if<Pred, Tuple<Types...>, source_seq, treat_seq>
    >::type;
};

template<template <class> class Pred, class Tuple, class Is>
using types_sequence_split_if_t = typename types_sequence_split_if<Pred, Tuple, Is>::type;

template<template <class> class Pred, class Tuple>
struct types_split_if
{
    using sequence_type = types_sequence_split_if_t<Pred, Tuple, make_types_index_sequence<Tuple>>;
    using left_type = reorder_types_t<Tuple, typename sequence_type::first_type>;
    using right_type = reorder_types_t<Tuple, typename sequence_type::second_type>;
    using type = std::pair<left_type, right_type>;
};

template<template <class> class Pred, class Tuple>
using types_split_if_t = typename types_split_if<Pred, Tuple>::type;

template<template <class> class Pred, class Tuple>
using types_split_if_left_t = typename types_split_if<Pred, Tuple>::left_type;

template<template <class> class Pred, class Tuple>
using types_split_if_right_t = typename types_split_if<Pred, Tuple>::right_type;


namespace private_detail_tuple_cat
{
    template <template <class...> class Tuple, class... Tuples>
    struct tuple_cat_helper_type;

    template <template <class...> class Tuple, class... Types>
    struct tuple_cat_helper_type<Tuple, Tuple<Types...>>
    {
        using type = Tuple<Types...>;
    };

    template <template <class...> class Tuple, class... Types0, class... Types1, class... Tuples>
    struct tuple_cat_helper_type<Tuple, Tuple<Types0...>, Tuple<Types1...>, Tuples...>
    {
        using type = typename tuple_cat_helper_type<Tuple, Tuple<Types0..., Types1...>, Tuples...>::type;
    };


    template <class... Tuples>
    struct tuple_cat_type;

    template <template <class...> class Tuple, class... Types, class... Tuples>
    struct tuple_cat_type<Tuple<Types...>, Tuples...>
    {
        using type = typename tuple_cat_helper_type<Tuple, Tuple<Types...>, Tuples...>::type;
    };

    template <class... Tuples>
    using tuple_cat_t = typename tuple_cat_type<Tuples...>::type;
}

using private_detail_tuple_cat::tuple_cat_type;
using private_detail_tuple_cat::tuple_cat_t;


template<template <class, class> class Cmp, class Tuple, class IS>
struct min_types_index_element_type;

template<template <class, class> class Cmp, class Tuple, size_t I0>
struct min_types_index_element_type<Cmp, Tuple, std::index_sequence<I0> > : std::integral_constant<size_t, I0>
{};

template<template <class, class> class Cmp, template <class...> class Tuple, class... Types, size_t I0, size_t I1, size_t... Indices>
struct min_types_index_element_type<Cmp, Tuple<Types...>, std::index_sequence<I0, I1, Indices...> > : std::conditional_t<
    Cmp<types_element_t<I0, Types...>, types_element_t<I1, Types...> >::value,
    min_types_index_element_type<Cmp, Tuple<Types...>, std::index_sequence<I0, Indices...> >,
    min_types_index_element_type<Cmp, Tuple<Types...>, std::index_sequence<I1, Indices...> >
>
{};

template<template <class, class> class Cmp, class Tuple, class IS = make_types_index_sequence<Tuple> >
constexpr auto min_types_index_element_v = min_types_index_element_type<Cmp, Tuple, IS>::value;


template<template <class, class> class Cmp, class Tuple, class IS>
struct sort_types_indices;

template<template <class, class> class Cmp, class Tuple>
struct sort_types_indices<Cmp, Tuple, std::index_sequence<> >
{
    using type = std::index_sequence<>;
};

template<template <class, class> class Cmp, class Tuple, size_t I0, size_t... Indices>
struct sort_types_indices<Cmp, Tuple, std::index_sequence<I0, Indices...> >
{
    using index_sequence_type = std::index_sequence<I0, Indices...>;
    static constexpr auto min_index_element_value = min_types_index_element_v<Cmp, Tuple, index_sequence_type>;

    using tail = types_pop_front_t<swap_index_sequence_t<I0, min_index_element_value, index_sequence_type>>;
    using sorted_tail = typename sort_types_indices<Cmp, Tuple, tail>::type;

    using type = index_sequence_push_front_t<
        min_index_element_value,
        sorted_tail
    >;
};

template<template <class, class> class Cmp, class Tuple, class IS = make_types_index_sequence<Tuple>>
using sort_types_indices_t = typename sort_types_indices<Cmp, Tuple, IS>::type;

template <template <class, class> class Cmp, class Tuple>
using sort_types_type = reorder_types<Tuple, sort_types_indices_t<Cmp, Tuple>>;

template <template <class, class> class Cmp, class Tuple>
using sort_types_t = typename sort_types_type<Cmp, Tuple>::type;


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
using tuple_sizeof_optimization_type = sort_types_type<greater_eq_sizeof, Tuple>;

template<class Tuple>
using tuple_sizeof_optimization_t = typename tuple_sizeof_optimization_type<Tuple>::type;


template<class Tuple, class T>
using tuple_unique_push_back_t = types_push_back_if_t<Tuple, std::negation_v<types_has_type<T, Tuple>>, T>;

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

template<class Tuple, template <class...> class TailTuple, class... Types>
struct tuple_unique_push_back_tuple_type<Tuple, TailTuple<Types...> >
{
    using type = tuple_unique_insert_back_t<Tuple, Types...>;
};

template<class Tuple, class TailTuple>
using tuple_unique_push_back_tuple_t = typename tuple_unique_push_back_tuple_type<Tuple, TailTuple>::type;

template<class Tuple>
struct unique_tuple_type;

template<template <class...> class Tuple, class... Types>
struct unique_tuple_type<Tuple<Types...>>
{
    using type = tuple_unique_push_back_tuple_t<Tuple<>, Tuple<Types...>>;
};

template<class Tuple>
using unique_tuple_t = typename unique_tuple_type<Tuple>::type;

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
