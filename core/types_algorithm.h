#pragma once

#include <core/invoke.h>



namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_function
    {
        template<class T>
        struct ttypes_function_helper;


        template<class R, class... Args>
        struct ttypes_function_helper<function_pointer_t<R, Args...>>
        {
            using type = ttypes<R, Args...>;
        };

        template<class T>
        using ttypes_function = ttypes_function_helper<remove_noexcept_t<std::decay_t<T> > >;

        template<class T>
        using ttypes_function_t = typename ttypes_function<T>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_function::ttypes_function;
using private_detail_types_algorithm::private_detail_ttypes_function::ttypes_function_t;


template<class T>
struct ttypes_size : index_constant<0u> {};

template<template <class...> class Tuple, class... Types>
struct ttypes_size<Tuple<Types...>> : index_constant<sizeof...(Types)> {};

template<class T>
constexpr size_t ttypes_size_v = ttypes_size<T>::value;


template<class T>
struct ttypes_sequence_size : index_constant<0u> {};

template<class SeqT, template <class, SeqT...> class Tuple, SeqT... Values>
struct ttypes_sequence_size<Tuple<SeqT, Values...>> : index_constant<sizeof...(Values)> {};

template<class T>
constexpr size_t ttypes_sequence_size_v = ttypes_sequence_size<T>::value;


template<template <class> class Pred, class Tuple>
struct ttypes_count_if;

template<template <class> class Pred, template <class...> class Tuple>
struct ttypes_count_if<Pred, Tuple<> > : index_constant<0_uz>
{};

template<template <class> class Pred, template <class...> class Tuple, class T, class... Types>
struct ttypes_count_if<Pred, Tuple<T, Types...> > : index_constant<
    static_cast<size_t>(Pred<T>::value) + ttypes_count_if<Pred, Tuple<Types...> >::value
>
{};

template<template <class> class Pred, class Tuple>
constexpr size_t ttypes_count_if_v = ttypes_count_if<Pred, Tuple>::value;


template<template <class...> class Left, template <class...> class Right>
using is_same_template = std::is_same<ttuples<Left>, ttuples<Right> >;

template<template <class...> class Left, template <class...> class Right>
constexpr bool is_same_template_v = is_same_template<Left, Right>::value;


template<class Source, template <class...> class Target>
struct template_is : std::false_type {};

template<template <class...> class Source, template <class...> class Target, class... Types>
struct template_is<Source<Types...>, Target> : is_same_template<Source, Target> {};

template<class Source, template <class...> class Target>
constexpr bool template_is_v = template_is<Source, Target>::value;

static_assert(template_is_v<dummy, ttypes>);

template<template <class...> class Tuple, class T>
using add_template_t = conditional_op_t<!template_is_v<T, Tuple>, Tuple, T>;


template <template <class...> class Pred, class Tuple, class... Args>
struct ttypes_has : std::false_type
{};

template <template <class...> class Pred, template <class...> class Tuple, class... Types, class... Args>
struct ttypes_has<Pred, Tuple<Types...>, Args...> : std::disjunction<Pred<Types, Args...>...>
{};

template <class T, class Tuple>
using ttypes_has_type = ttypes_has<std::is_same, Tuple, T>;

template <class Tuple, class... Args>
using ttypes_has_invoke = ttypes_has<is_invocable, Tuple, Args...>;

template<template <class...> class Pred, class Tuple, class... Args>
constexpr bool ttypes_has_v = ttypes_has<Pred, Tuple, Args...>::value;

template <class T, class Tuple>
constexpr bool ttypes_has_type_v = ttypes_has_type<T, Tuple>::value;

template <class Tuple, class... Args>
constexpr bool ttypes_has_invoke_v = ttypes_has_invoke<Tuple, Args...>::value;


namespace private_detail_types_algorithm
{
    namespace private_detail_subtypes
    {
        struct ttypes_function
        {
            template<class... Args>
            constexpr ttypes<Args...> operator () (const Args&...) const noexcept
            {
                return {};
            }
        };

        template<class T>
        using subtypes_t = subapply_result_t<T, ttypes_function>;
    }
}

using private_detail_types_algorithm::private_detail_subtypes::subtypes_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_repack
    {
        template<class Tuple, template <class...> class NewTuple>
        struct ttypes_repack;

        template<template <class...> class Tuple, template <class...> class NewTuple, class... Types>
        struct ttypes_repack<Tuple<Types...>, NewTuple>
        {
            using type = NewTuple<Types...>;
        };

        template<class Tuple, template <class...> class NewTuple>
        using ttypes_repack_t = typename ttypes_repack<Tuple, NewTuple>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_repack::ttypes_repack_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_rewrite
    {
        template<class Tuple, class... NewTypes>
        struct ttypes_rewrite;

        template<template <class...> class Tuple, class... Types, class... NewTypes>
        struct ttypes_rewrite<Tuple<Types...>, NewTypes...>
        {
            using type = Tuple<NewTypes...>;
        };

        template<class Tuple, class... NewTypes>
        using ttypes_rewrite_t = typename ttypes_rewrite<Tuple, NewTypes...>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_rewrite::ttypes_rewrite_t;

template<class Tuple>
using ttypes_clear_t = ttypes_rewrite_t<Tuple>;


namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_transform
    {
        template<template <class> class Fn, class Tuple>
        struct ttypes_transform
        {
            using type = Tuple;
        };

        template<template <class> class Fn, template <class...> class Tuple, class... Types>
        struct ttypes_transform<Fn, Tuple<Types...>>
        {
            using type = Tuple<Fn<Types>...>;
        };

        template<template <class> class Fn, class Tuple>
        using ttypes_transform_t = typename ttypes_transform<Fn, Tuple>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_transform::ttypes_transform_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_swap_index_sequence
    {
        template <size_t I, size_t J, class IS>
        struct swap_index_sequence;

        template <size_t I, size_t J, size_t... Indices>
        struct swap_index_sequence<I, J, std::index_sequence<Indices...>>
        {
            using type = std::index_sequence<((Indices != I) ? ((Indices != J) ? Indices : I) : J)...>;
        };

        template <size_t I, size_t J, class IS>
        using swap_index_sequence_t = typename swap_index_sequence<I, J, IS>::type;
    }

    namespace private_detail_index_sequence_pop_front
    {
        template<class IS>
        struct index_sequence_pop_front;

        template <size_t I0, size_t... Indices>
        struct index_sequence_pop_front<std::index_sequence<I0, Indices...> >
        {
            using type = std::index_sequence<Indices...>;
        };

        template <class IS>
        using index_sequence_pop_front_t = typename index_sequence_pop_front<IS>::type;
    }

    namespace private_detail_index_sequence_push_front
    {
        template<size_t I0, class IS>
        struct index_sequence_push_front;

        template <size_t I0, size_t... Indices>
        struct index_sequence_push_front<I0, std::index_sequence<Indices...> >
        {
            using type = std::index_sequence<I0, Indices...>;
        };

        template <size_t I0, class IS>
        using index_sequence_push_front_t = typename index_sequence_push_front<I0, IS>::type;
    }
}

using private_detail_types_algorithm::private_detail_swap_index_sequence::swap_index_sequence_t;
using private_detail_types_algorithm::private_detail_index_sequence_pop_front::index_sequence_pop_front_t;
using private_detail_types_algorithm::private_detail_index_sequence_push_front::index_sequence_push_front_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_type_map
    {
        template<class... Types>
        struct type_map : Types...
        {
            using Types::decl...;
        };

        template<size_t Index, class Map>
        using types_map_element_t = decltype(Map::decl(index_constant_v<Index>));
    }

    namespace private_detail_ttypes_element
    {
        using namespace private_detail_type_map;

        template<size_t Index, class T>
        struct types_map_element
        {
            static T decl(index_constant<Index>);
        };

        template <class IS, class... Types>
        struct make_types_map_helper;

        template <size_t... Indices, class... Types>
        struct make_types_map_helper<std::index_sequence<Indices...>, Types...>
        {
            using type = type_map<types_map_element<Indices, Types>...>;
        };

        template<class... Types>
        using make_types_map_t = typename make_types_map_helper<std::make_index_sequence<sizeof...(Types)>, Types...>::type;

        template<size_t Index, class Map>
        using types_map_element_t = decltype(Map::decl(index_constant_v<Index>));

        template <size_t Index, class... Types>
        using types_element_t = types_map_element_t<Index, make_types_map_t<Types...>>;

        template<size_t Index, class Tuple>
        struct ttypes_element_type;

        template<size_t Index, template <class...> class Tuple, class... Types>
        struct ttypes_element_type<Index, Tuple<Types...>>
        {
            using type = types_element_t<Index, Types...>;
        };

        template<size_t Index, class Tuple>
        using ttypes_element_t = typename ttypes_element_type<Index, Tuple>::type;
    }

    namespace private_detail_ttypes_index
    {
        using namespace private_detail_type_map;

        template<size_t Index, class T>
        struct ttypes_index_map_element
        {
            static index_constant<Index> decl(ttypes<T>);
        };

        template <class IS, class... Types>
        struct make_ttypes_index_map_helper;

        template <size_t... Indices, class... Types>
        struct make_ttypes_index_map_helper<std::index_sequence<Indices...>, Types...>
        {
            using type = type_map<ttypes_index_map_element<Indices, Types>...>;
        };

        template<class... Types>
        using make_ttypes_index_map_t = typename make_ttypes_index_map_helper<std::make_index_sequence<sizeof...(Types)>, Types...>::type;

        template<class T, class Map>
        using ttypes_map_index_element_t = decltype(Map::decl(ttypes_v<T>));

        template <class T, class... Types>
        using types_index_element_t = ttypes_map_index_element_t<T, make_ttypes_index_map_t<Types...>>;

        template <class T, class... Types>
        constexpr auto types_index_element_v = types_index_element_t<T, Types...>::value;

        template<class T, class Tuple>
        struct ttypes_index_element_type;

        template<class T, template <class...> class Tuple, class... Types>
        struct ttypes_index_element_type<T, Tuple<Types...>>
        {
            using type = types_index_element_t<T, Types...>;
        };

        template <class T, class Tuple>
        using ttypes_index_element_t = typename ttypes_index_element_type<T, Tuple>::type;

        template <class T, class Tuple>
        constexpr auto ttypes_index_element_v = ttypes_index_element_t<T, Tuple>::value;
    }

    namespace private_detail_tuples_element
    {
        using namespace private_detail_type_map;

        template<size_t Index, template <class...> class Tuple>
        struct tuples_map_element
        {
            static ttuples<Tuple> decl(index_constant<Index>);
        };

        template <class IS, template <class...> class... Tuples>
        struct make_tuples_map_helper;

        template <size_t... Indices, template <class...> class... Tuples>
        struct make_tuples_map_helper<std::index_sequence<Indices...>, Tuples...>
        {
            using type = type_map<tuples_map_element<Indices, Tuples>...>;
        };

        template<template <class...> class... Tuples>
        using make_tuples_map_t = typename make_tuples_map_helper<std::make_index_sequence<sizeof...(Tuples)>, Tuples...>::type;

        template <size_t Index, template <class...> class... Tuples>
        using tuples_element_t = types_map_element_t<Index, make_tuples_map_t<Tuples...>>;

        template<class Pack, class... Types>
        struct make_ttuple_type;

        template<template <class...> class Tuple, class... Types>
        struct make_ttuple_type<ttuples<Tuple>, Types...>
        {
            using type = Tuple<Types...>;
        };

        template<class Pack, class... Types>
        using make_ttuple_t = typename make_ttuple_type<Pack, Types...>::type;

        template<size_t Index, class TuplesPack>
        struct ttuples_element_type;

        template<size_t Index, template <class...> class... Tuples>
        struct ttuples_element_type<Index, ttuples<Tuples...>>
        {
            using type = tuples_element_t<Index, Tuples...>;
        };

        template<size_t Index, class TuplesPack>
        using ttuples_element_t = typename ttuples_element_type<Index, TuplesPack>::type;

        template<size_t Index, class TuplesPack, class... Types>
        using make_ttuples_element_t = make_ttuple_t<ttuples_element_t<Index, TuplesPack>, Types...>;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_element::make_types_map_t;
using private_detail_types_algorithm::private_detail_ttypes_element::types_map_element_t;
using private_detail_types_algorithm::private_detail_ttypes_element::types_element_t;
using private_detail_types_algorithm::private_detail_ttypes_element::ttypes_element_t;
using private_detail_types_algorithm::private_detail_ttypes_index::types_index_element_v;
using private_detail_types_algorithm::private_detail_ttypes_index::ttypes_index_element_v;
using private_detail_types_algorithm::private_detail_tuples_element::tuples_element_t;
using private_detail_types_algorithm::private_detail_tuples_element::make_ttuples_element_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_reorder
    {
        template<class Tuple, class Is>
        struct ttypes_reorder;

        template<template <class...> class Tuple, class... Types, size_t... Indices>
        struct ttypes_reorder<Tuple<Types...>, std::index_sequence<Indices...> >
        {
            using types_map_type = make_types_map_t<Types...>;

            using type = Tuple<types_map_element_t<Indices, types_map_type>...>;
        };

        template<class Tuple, class Is>
        using ttypes_reorder_t = typename ttypes_reorder<Tuple, Is>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_reorder::ttypes_reorder_t;

template <size_t I, size_t J, class Tuple>
using types_swap_t = ttypes_reorder_t<
    Tuple,
    swap_index_sequence_t<I, J, std::make_index_sequence<ttypes_size_v<Tuple>>>
>;

namespace private_detail_types_algorithm
{
    namespace private_detail_types_front_or
    {
        template <class Default, class... Types>
        struct types_front_or;

        template <class Default>
        struct types_front_or<Default>
        {
            using type = Default;
        };

        template <class Default, class T, class... Types>
        struct types_front_or<Default, T, Types...>
        {
            using type = T;
        };

        template <class Default, class... Types>
        using types_front_or_t = typename types_front_or<Default, Types...>::type;
    }

    namespace private_detail_ttypes_pop_front
    {
        template <class Tuple>
        struct ttypes_pop_front;

        template <template <class...> class Tuple, class T, class... Types>
        struct ttypes_pop_front<Tuple<T, Types...>>
        {
            using type = Tuple<Types...>;
        };

        template <class Tuple>
        using ttypes_pop_front_t = typename ttypes_pop_front<Tuple>::type;
    }

    namespace private_detail_ttypes_push_front
    {
        template <class T, class Tuple>
        struct ttypes_push_front;

        template <class T, template <class...> class Tuple, class... Types>
        struct ttypes_push_front<T, Tuple<Types...>>
        {
            using type = Tuple<T, Types...>;
        };

        template <class T, class Tuple>
        using ttypes_push_front_t = typename ttypes_push_front<T, Tuple>::type;
    }

    namespace private_detail_ttypes_insert_back
    {
        template <class Tuple, class... Types>
        struct ttypes_insert_back;

        template <template <class...> class Tuple, class... TupleTypes, class... Types>
        struct ttypes_insert_back<Tuple<TupleTypes...>, Types...>
        {
            using type = Tuple<TupleTypes..., Types...>;
        };

        template <class Tuple, class... Types>
        using ttypes_insert_back_t = typename ttypes_insert_back<Tuple, Types...>::type;
    }

    namespace private_detail_ttypes_push_back_ttypes
    {
        template <class Tuple, class Pack>
        struct ttypes_push_back_ttypes;

        template <template <class...> class Tuple, class... TupleTypes, class... Types>
        struct ttypes_push_back_ttypes<Tuple<TupleTypes...>, Tuple<Types...>>
        {
            using type = Tuple<TupleTypes..., Types...>;
        };

        template <class Tuple, class... Types>
        using ttypes_push_back_ttypes_t = typename ttypes_push_back_ttypes<Tuple, Types...>::type;
    }
}

using private_detail_types_algorithm::private_detail_types_front_or::types_front_or_t;
using private_detail_types_algorithm::private_detail_ttypes_pop_front::ttypes_pop_front_t;
using private_detail_types_algorithm::private_detail_ttypes_push_front::ttypes_push_front_t;
using private_detail_types_algorithm::private_detail_ttypes_insert_back::ttypes_insert_back_t;
using private_detail_types_algorithm::private_detail_ttypes_push_back_ttypes::ttypes_push_back_ttypes_t;

template <class Tuple>
using ttypes_pop_back_t = ttypes_reorder_t<Tuple, std::make_index_sequence<ttypes_size_v<Tuple> -1_uz> >;

template<class Tuple, bool test, class... Types>
using ttypes_insert_back_if_t = conditional_op_or_t<test, Tuple, ttypes_insert_back_t, Tuple, Types...>;


namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_select
    {
        template <template <class...> class Pred, class Tuple0, class Tuple, class... Args>
        struct ttypes_select_helper;

        template <template <class...> class Pred, template <class...> class Tuple, class... Types0, class... Args>
        struct ttypes_select_helper<Pred, Tuple<Types0...>, Tuple<>, Args...>
        {
            using type = Tuple<Types0...>;
        };

        template <template <class...> class Pred, template <class...> class Tuple, class... Types0, class T, class... Types, class... Args>
        struct ttypes_select_helper<Pred, Tuple<Types0...>, Tuple<T, Types...>, Args...>
        {
            using type = typename ttypes_select_helper<
                Pred,
                std::conditional_t<Pred<T, Args...>::value, Tuple<Types0..., T>, Tuple<Types0...> >,
                Tuple<Types...>,
                Args...
            >::type;
        };

        template <template <class...> class Pred, class Tuple, class... Args>
        using ttypes_select_t = typename ttypes_select_helper<Pred, ttypes_clear_t<Tuple>, Tuple, Args...>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_select::ttypes_select_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_types_cat
    {
        template<template <class...> class Tuple, class L, class R>
        struct types_pairing_for
        {
            using type = Tuple<L, R>;
        };

        template<template <class...> class Tuple, class L, class... RTypes>
        struct types_pairing_for<Tuple, L, Tuple<RTypes...> >
        {
            using type = Tuple<L, RTypes...>;
        };

        template<template <class...> class Tuple, class... LTypes, class R>
        struct types_pairing_for<Tuple, Tuple<LTypes...>, R>
        {
            using type = Tuple<LTypes..., R>;
        };

        template<template <class...> class Tuple, class... LTypes, class... RTypes>
        struct types_pairing_for<Tuple, Tuple<LTypes...>, Tuple<RTypes...> >
        {
            using type = Tuple<LTypes..., RTypes...>;
        };

        template<template <class...> class Tuple, class L, class R>
        using types_pairing_for_t = typename types_pairing_for<Tuple, L, R>::type;


        template <template <class...> class Tuple, class... Types>
        struct types_cat_for;

        template <template <class...> class Tuple>
        struct types_cat_for<Tuple>
        {
            using type = Tuple<>;
        };

        template <template <class...> class Tuple, class T>
        struct types_cat_for<Tuple, T>
        {
            using type = add_template_t<Tuple, T>;
        };

        template <template <class...> class Tuple, class T0, class T1, class... TailTypes>
        struct types_cat_for<Tuple, T0, T1, TailTypes...>
        {
            using type = ttypes_push_back_ttypes_t<types_pairing_for_t<Tuple, T0, T1>, typename types_cat_for<Tuple, TailTypes...>::type>;
        };

        template <template <class...> class Tuple, class... Tuples>
        using types_cat_for_t = typename types_cat_for<Tuple, Tuples...>::type;

        template <class... Types>
        using types_cat_t = types_cat_for_t<ttypes, Types...>;
    }
}

using private_detail_types_algorithm::private_detail_types_cat::types_cat_for_t;
using private_detail_types_algorithm::private_detail_types_cat::types_cat_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_types_sol
    {
        template<class Tuple>
        struct types_sol;

        template<template <class...> class Tuple, class... Types>
        struct types_sol<Tuple<Types...> >
        {
            using type = types_cat_for_t<Tuple, Types...>;
        };

        template<class T>
        using types_sol_t = typename types_sol<T>::type;
    }
}

using private_detail_types_algorithm::private_detail_types_sol::types_sol_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_min_ttypes_index_element
    {
        template<template <class, class> class Cmp, class Tuple, class IS>
        struct min_ttypes_index_element;

        template<template <class, class> class Cmp, class Tuple, size_t I0>
        struct min_ttypes_index_element<Cmp, Tuple, std::index_sequence<I0> > : std::integral_constant<size_t, I0>
        {};

        template<template <class, class> class Cmp, template <class...> class Tuple, class... Types, size_t I0, size_t I1, size_t... Indices>
        struct min_ttypes_index_element<Cmp, Tuple<Types...>, std::index_sequence<I0, I1, Indices...> > : std::conditional_t<
            Cmp<types_element_t<I0, Types...>, types_element_t<I1, Types...> >::value,
            min_ttypes_index_element<Cmp, Tuple<Types...>, std::index_sequence<I0, Indices...> >,
            min_ttypes_index_element<Cmp, Tuple<Types...>, std::index_sequence<I1, Indices...> >
        >
        {};

        template<template <class, class> class Cmp, class Tuple, class IS = std::make_index_sequence<ttypes_size_v<Tuple>> >
        constexpr auto min_ttypes_index_element_v = min_ttypes_index_element<Cmp, Tuple, IS>::value;
    }
}

using private_detail_types_algorithm::private_detail_min_ttypes_index_element::min_ttypes_index_element_v;


namespace private_detail_types_algorithm
{
    namespace private_detail_sort_ttypes_indices
    {
        template<template <class, class> class Cmp, class Tuple, class IS>
        struct sort_ttypes_indices;

        template<template <class, class> class Cmp, class Tuple>
        struct sort_ttypes_indices<Cmp, Tuple, std::index_sequence<> >
        {
            using type = std::index_sequence<>;
        };

        template<template <class, class> class Cmp, class Tuple, size_t I0, size_t... Indices>
        struct sort_ttypes_indices<Cmp, Tuple, std::index_sequence<I0, Indices...> >
        {
            using index_sequence_type = std::index_sequence<I0, Indices...>;
            static constexpr auto min_index_element_value = min_ttypes_index_element_v<Cmp, Tuple, index_sequence_type>;

            using tail = index_sequence_pop_front_t<swap_index_sequence_t<I0, min_index_element_value, index_sequence_type>>;
            using sorted_tail = typename sort_ttypes_indices<Cmp, Tuple, tail>::type;

            using type = index_sequence_push_front_t<
                min_index_element_value,
                sorted_tail
            >;
        };

        template<template <class, class> class Cmp, class Tuple, class IS = std::make_index_sequence<ttypes_size_v<Tuple>>>
        using sort_ttypes_indices_t = typename sort_ttypes_indices<Cmp, Tuple, IS>::type;
    }
}

using private_detail_types_algorithm::private_detail_sort_ttypes_indices::sort_ttypes_indices_t;


template <template <class, class> class Cmp, class Tuple>
using sort_ttypes_t = ttypes_reorder_t<Tuple, sort_ttypes_indices_t<Cmp, Tuple>>;


template <class T, class U>
constexpr bool less_sizeof_v = less_op(sizeof(T), sizeof(U));

template <class T, class U>
constexpr bool greater_sizeof_v = less_sizeof_v<U, T>;

template <class T, class U>
constexpr bool less_eq_sizeof_v = !less_sizeof_v<U, T>;

template <class T, class U>
constexpr bool greater_eq_sizeof_v = !less_sizeof_v<T, U>;

template <class T, class U>
using less_sizeof = std::bool_constant<less_sizeof_v<T, U>>;

template <class T, class U>
using greater_sizeof = std::bool_constant<greater_sizeof_v<T, U>>;

template <class T, class U>
using less_eq_sizeof = std::bool_constant<less_eq_sizeof_v<T, U>>;

template <class T, class U>
using greater_eq_sizeof = std::bool_constant<greater_eq_sizeof_v<T, U>>;


template<class Tuple>
using ttypes_sizeof_optimization_t = sort_ttypes_t<greater_eq_sizeof, Tuple>;

template<class Tuple, class T>
using ttypes_unique_push_back_t = ttypes_insert_back_if_t<Tuple, !ttypes_has_type_v<T, Tuple>, T>;


namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_unique_insert_back
    {
        template<class Tuple, class... Types>
        struct ttypes_unique_insert_back;

        template<class Tuple>
        struct ttypes_unique_insert_back<Tuple>
        {
            using type = Tuple;
        };

        template<class Tuple, class T0, class... Types>
        struct ttypes_unique_insert_back<Tuple, T0, Types...>
        {
            using type = typename ttypes_unique_insert_back<
                ttypes_unique_push_back_t<Tuple, T0>,
                Types...
            >::type;
        };

        template<class Tuple, class... Types>
        using ttypes_unique_insert_back_t = typename ttypes_unique_insert_back<Tuple, Types...>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_unique_insert_back::ttypes_unique_insert_back_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_unique_push_back_ttypes
    {
        template<template <class...> class Tuple, class Pack, class TailPack>
        struct ttypes_unique_push_back_ttypes;

        template<template <class...> class Tuple, class... Types, class... Tail>
        struct ttypes_unique_push_back_ttypes<Tuple, Tuple<Types...>, Tuple<Tail...> >
        {
            using type = ttypes_unique_insert_back_t<Tuple<Types...>, Tail...>;
        };

        template<template <class...> class Tuple, class Pack, class TailPack>
        using ttypes_unique_push_back_ttypes_t = typename ttypes_unique_push_back_ttypes<Tuple, Pack, TailPack>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_unique_push_back_ttypes::ttypes_unique_push_back_ttypes_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_ttypes_unique
    {
        template<class Tuple>
        struct ttypes_unique;

        template<template <class...> class Tuple, class... Types>
        struct ttypes_unique<Tuple<Types...>>
        {
            using type = ttypes_unique_insert_back_t<Tuple<>, Types...>;
        };

        template<class Tuple>
        using ttypes_unique_t = typename ttypes_unique<Tuple>::type;
    }
}

using private_detail_types_algorithm::private_detail_ttypes_unique::ttypes_unique_t;