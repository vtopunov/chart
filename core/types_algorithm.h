#pragma once

#include <core/utility.h>


template<class T>
struct types_size : index_constant<0u> {};

template<template <class...> class Tuple, class... Types>
struct types_size<Tuple<Types...>> : index_constant<sizeof...(Types)> {};

template<class T>
constexpr size_t types_size_v = types_size<T>::value;


template<class T>
struct types_sequence_size : index_constant<0u> {};

template<class SeqT, template <class, SeqT...> class Tuple, SeqT... Values>
struct types_sequence_size<Tuple<SeqT, Values...>> : index_constant<sizeof...(Values)> {};

template<class T>
constexpr size_t types_sequence_size_v = types_sequence_size<T>::value;


template<template <class> class Pred, class Tuple>
struct types_count_if;

template<template <class> class Pred, template <class...> class Tuple>
struct types_count_if<Pred, Tuple<> > : index_constant<0_uz>
{};

template<template <class> class Pred, template <class...> class Tuple, class T, class... Types>
struct types_count_if<Pred, Tuple<T, Types...> > : index_constant<
    static_cast<size_t>(Pred<T>::value) + types_count_if<Pred, Tuple<Types...> >::value
>
{};

template<template <class> class Pred, class Tuple>
constexpr size_t types_count_if_v = types_count_if<Pred, Tuple>::value;


template<template <class...> class Left, template <class...> class Right>
using is_same_types_template = std::is_same<tuple_pack<Left>, tuple_pack<Right> >;

template<template <class...> class Left, template <class...> class Right>
constexpr bool is_same_types_template_v = is_same_types_template<Left, Right>::value;


template<class Source, template <class...> class Target>
struct types_template_is : std::false_type {};

template<template <class...> class Source, template <class...> class Target, class... Types>
struct types_template_is<Source<Types...>, Target> : is_same_types_template<Source, Target> {};

template<class Source, template <class...> class Target>
constexpr bool types_template_is_v = types_template_is<Source, Target>::value;


template<template <class...> class Tuple, class T>
using add_template_t = conditional_op_t<!types_template_is_v<T, Tuple>, Tuple, T>;


template <template <class...> class Pred, class Tuple, class... Args>
struct types_has : std::false_type
{};

template <template <class...> class Pred, template <class...> class Tuple, class... Types, class... Args>
struct types_has<Pred, Tuple<Types...>, Args...> : std::disjunction<Pred<Types, Args...>...>
{};

template <class T, class Tuple>
using types_has_type = types_has<std::is_same, Tuple, T>;

template <class Tuple, class... Args>
using types_has_call = types_has<call_is_detected, Tuple, Args...>;

template<template <class...> class Pred, class Tuple, class... Args>
constexpr bool types_has_v = types_has<Pred, Tuple, Args...>::value;

template <class T, class Tuple>
constexpr bool types_has_type_v = types_has_type<T, Tuple>::value;

template <class Tuple, class... Args>
constexpr bool types_has_call_v = types_has_call<Tuple, Args...>::value;


namespace private_detail_types_algorithm
{
    namespace private_detail_subtypes
    {
        static_assert(types_template_is_v<noapply_t, types_pack>);

        struct types_pack_function
        {
            template<class... Args>
            constexpr types_pack<Args...> operator () (const Args&...) const noexcept
            {
                return {};
            }
        };

        template<class T>
        using subtypes_t = subapply_result_t<T, types_pack_function>;
    }
}

using private_detail_types_algorithm::private_detail_subtypes::subtypes_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_repack_types
    {
        template<class Tuple, template <class...> class NewTuple>
        struct repack_types;

        template<template <class...> class Tuple, template <class...> class NewTuple, class... Types>
        struct repack_types<Tuple<Types...>, NewTuple>
        {
            using type = NewTuple<Types...>;
        };

        template<class Tuple, template <class...> class NewTuple>
        using repack_types_t = typename repack_types<Tuple, NewTuple>::type;
    }
}

using private_detail_types_algorithm::private_detail_repack_types::repack_types_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_rewrite_types
    {
        template<class Tuple, class... NewTypes>
        struct rewrite_types;

        template<template <class...> class Tuple, class... Types, class... NewTypes>
        struct rewrite_types<Tuple<Types...>, NewTypes...>
        {
            using type = Tuple<NewTypes...>;
        };

        template<class Tuple, class... NewTypes>
        using rewrite_types_t = typename rewrite_types<Tuple, NewTypes...>::type;
    }
}

using private_detail_types_algorithm::private_detail_rewrite_types::rewrite_types_t;

template<class Tuple>
using clear_types_t = rewrite_types_t<Tuple>;


namespace private_detail_types_algorithm
{
    namespace private_detail_transform_types
    {
        template<template <class> class Fn, class Tuple>
        struct transform_types
        {
            using type = Tuple;
        };

        template<template <class> class Fn, template <class...> class Tuple, class... Types>
        struct transform_types<Fn, Tuple<Types...>>
        {
            using type = Tuple<Fn<Types>...>;
        };

        template<template <class> class Fn, class Tuple>
        using transform_types_t = typename transform_types<Fn, Tuple>::type;
    }
}

using private_detail_types_algorithm::private_detail_transform_types::transform_types_t;


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

    namespace private_detail_types_element
    {
        using namespace private_detail_type_map;

        template<size_t Index, class T>
        struct types_type_element
        {
            static T decl(index_constant<Index>);
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

        template<size_t Index, class Tuple>
        struct types_pack_element_type;

        template<size_t Index, template <class...> class Tuple, class... Types>
        struct types_pack_element_type<Index, Tuple<Types...>>
        {
            using type = types_element_t<Index, Types...>;
        };

        template<size_t Index, class Tuple>
        using types_pack_element_t = typename types_pack_element_type<Index, Tuple>::type;
    }

    namespace private_detail_tuples_element
    {
        using namespace private_detail_type_map;

        template<size_t Index, template <class...> class Tuple>
        struct element_tuple_pack
        {
            static tuple_pack<Tuple> decl(index_constant<Index>);
        };

        template <class IS, template <class...> class... Tuples>
        struct make_tuples_map_helper;

        template <size_t... Indices, template <class...> class... Tuples>
        struct make_tuples_map_helper<std::index_sequence<Indices...>, Tuples...>
        {
            using type = type_map<element_tuple_pack<Indices, Tuples>...>;
        };

        template<template <class...> class... Tuples>
        using make_tuples_map_t = typename make_tuples_map_helper<std::make_index_sequence<sizeof...(Tuples)>, Tuples...>::type;

        template <size_t Index, template <class...> class... Tuples>
        using tuples_element_tuple_pack_t = types_map_element_t<Index, make_tuples_map_t<Tuples...>>;

        template<class Pack, class... Types>
        struct make_tuple_pack_type;

        template<template <class...> class Tuple, class... Types>
        struct make_tuple_pack_type<tuple_pack<Tuple>, Types...>
        {
            using type = Tuple<Types...>;
        };

        template<class Pack, class... Types>
        using make_tuple_pack_t = typename make_tuple_pack_type<Pack, Types...>::type;

        template<size_t Index, class TuplesPack>
        struct tuples_pack_element_tuple_pack_type;

        template<size_t Index, template <class...> class... Tuples>
        struct tuples_pack_element_tuple_pack_type<Index, tuples_pack<Tuples...>>
        {
            using type = tuples_element_tuple_pack_t<Index, Tuples...>;
        };

        template<size_t Index, class TuplesPack>
        using tuples_pack_element_tuple_pack_t = typename tuples_pack_element_tuple_pack_type<Index, TuplesPack>::type;

        template<size_t Index, class TuplesPack, class... Types>
        using make_tuples_pack_element_t = make_tuple_pack_t<tuples_pack_element_tuple_pack_t<Index, TuplesPack>, Types...>;
    }
}

using private_detail_types_algorithm::private_detail_types_element::make_types_map_t;
using private_detail_types_algorithm::private_detail_types_element::types_map_element_t;
using private_detail_types_algorithm::private_detail_types_element::types_element_t;
using private_detail_types_algorithm::private_detail_types_element::types_pack_element_t;
using private_detail_types_algorithm::private_detail_tuples_element::tuples_element_tuple_pack_t;
using private_detail_types_algorithm::private_detail_tuples_element::make_tuples_pack_element_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_reorder_types
    {
        template<class Tuple, class Is>
        struct reorder_types;

        template<template <class...> class Tuple, class... Types, size_t... Indices>
        struct reorder_types<Tuple<Types...>, std::index_sequence<Indices...> >
        {
            using types_map_type = make_types_map_t<Types...>;

            using type = Tuple<types_map_element_t<Indices, types_map_type>...>;
        };

        template<class Tuple, class Is>
        using reorder_types_t = typename reorder_types<Tuple, Is>::type;
    }
}

using private_detail_types_algorithm::private_detail_reorder_types::reorder_types_t;

template <size_t I, size_t J, class Tuple>
using types_swap_t = reorder_types_t<
    Tuple,
    swap_index_sequence_t<I, J, std::make_index_sequence<types_size_v<Tuple>>>
>;

namespace private_detail_types_algorithm
{
    namespace private_detail_types_pop_front
    {
        template <class Tuple>
        struct types_pop_front;

        template <template <class...> class Tuple, class T, class... Types>
        struct types_pop_front<Tuple<T, Types...>>
        {
            using type = Tuple<Types...>;
        };

        template <class Tuple>
        using types_pop_front_t = typename types_pop_front<Tuple>::type;
    }

    namespace private_detail_types_push_front
    {
        template <class T, class Tuple>
        struct types_push_front;

        template <class T, template <class...> class Tuple, class... Types>
        struct types_push_front<T, Tuple<Types...>>
        {
            using type = Tuple<T, Types...>;
        };

        template <class T, class Tuple>
        using types_push_front_t = typename types_push_front<T, Tuple>::type;
    }

    namespace private_detail_types_insert_back
    {
        template <class Tuple, class... Types>
        struct types_insert_back;

        template <template <class...> class Tuple, class... TupleTypes, class... Types>
        struct types_insert_back<Tuple<TupleTypes...>, Types...>
        {
            using type = Tuple<TupleTypes..., Types...>;
        };

        template <class Tuple, class... Types>
        using types_insert_back_t = typename types_insert_back<Tuple, Types...>::type;
    }

    namespace private_detail_types_push_back_pack
    {
        template <class Tuple, class Pack>
        struct types_push_back_pack;

        template <template <class...> class Tuple, class... TupleTypes, class... Types>
        struct types_push_back_pack<Tuple<TupleTypes...>, Tuple<Types...>>
        {
            using type = Tuple<TupleTypes..., Types...>;
        };

        template <class Tuple, class... Types>
        using types_push_back_pack_t = typename types_push_back_pack<Tuple, Types...>::type;
    }
}

using private_detail_types_algorithm::private_detail_types_pop_front::types_pop_front_t;
using private_detail_types_algorithm::private_detail_types_push_front::types_push_front_t;
using private_detail_types_algorithm::private_detail_types_insert_back::types_insert_back_t;
using private_detail_types_algorithm::private_detail_types_push_back_pack::types_push_back_pack_t;

template <class Tuple>
using types_pop_back_t = reorder_types_t<Tuple, std::make_index_sequence<types_size_v<Tuple> -1_uz> >;

template<class Tuple, bool test, class... Types>
using types_insert_back_if_t = conditional_op_or_t<test, Tuple, types_insert_back_t, Tuple, Types...>;


namespace private_detail_types_algorithm
{
    namespace private_detail_types_select
    {
        template <template <class...> class Pred, class Tuple0, class Tuple, class... Args>
        struct types_select_helper;

        template <template <class...> class Pred, template <class...> class Tuple, class... Types0, class... Args>
        struct types_select_helper<Pred, Tuple<Types0...>, Tuple<>, Args...>
        {
            using type = Tuple<Types0...>;
        };

        template <template <class...> class Pred, template <class...> class Tuple, class... Types0, class T, class... Types, class... Args>
        struct types_select_helper<Pred, Tuple<Types0...>, Tuple<T, Types...>, Args...>
        {
            using type = typename types_select_helper<
                Pred,
                std::conditional_t<Pred<T, Args...>::value, Tuple<Types0..., T>, Tuple<Types0...> >,
                Tuple<Types...>,
                Args...
            >::type;
        };

        template <template <class...> class Pred, class Tuple, class... Args>
        using types_select_t = typename types_select_helper<Pred, clear_types_t<Tuple>, Tuple, Args...>::type;
    }
}

using private_detail_types_algorithm::private_detail_types_select::types_select_t;


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
            using type = types_push_back_pack_t<types_pairing_for_t<Tuple, T0, T1>, typename types_cat_for<Tuple, TailTypes...>::type>;
        };

        template <template <class...> class Tuple, class... Tuples>
        using types_cat_for_t = typename types_cat_for<Tuple, Tuples...>::type;

        template <class... Types>
        using types_cat_t = types_cat_for_t<types_pack, Types...>;
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
    namespace private_detail_min_types_index_element
    {
        template<template <class, class> class Cmp, class Tuple, class IS>
        struct min_types_index_element;

        template<template <class, class> class Cmp, class Tuple, size_t I0>
        struct min_types_index_element<Cmp, Tuple, std::index_sequence<I0> > : std::integral_constant<size_t, I0>
        {};

        template<template <class, class> class Cmp, template <class...> class Tuple, class... Types, size_t I0, size_t I1, size_t... Indices>
        struct min_types_index_element<Cmp, Tuple<Types...>, std::index_sequence<I0, I1, Indices...> > : std::conditional_t<
            Cmp<types_element_t<I0, Types...>, types_element_t<I1, Types...> >::value,
            min_types_index_element<Cmp, Tuple<Types...>, std::index_sequence<I0, Indices...> >,
            min_types_index_element<Cmp, Tuple<Types...>, std::index_sequence<I1, Indices...> >
        >
        {};

        template<template <class, class> class Cmp, class Tuple, class IS = std::make_index_sequence<types_size_v<Tuple>> >
        constexpr auto min_types_index_element_v = min_types_index_element<Cmp, Tuple, IS>::value;
    }
}

using private_detail_types_algorithm::private_detail_min_types_index_element::min_types_index_element_v;


namespace private_detail_types_algorithm
{
    namespace private_detail_sort_types_indices
    {
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

            using tail = index_sequence_pop_front_t<swap_index_sequence_t<I0, min_index_element_value, index_sequence_type>>;
            using sorted_tail = typename sort_types_indices<Cmp, Tuple, tail>::type;

            using type = index_sequence_push_front_t<
                min_index_element_value,
                sorted_tail
            >;
        };

        template<template <class, class> class Cmp, class Tuple, class IS = std::make_index_sequence<types_size_v<Tuple>>>
        using sort_types_indices_t = typename sort_types_indices<Cmp, Tuple, IS>::type;
    }
}

using private_detail_types_algorithm::private_detail_sort_types_indices::sort_types_indices_t;


template <template <class, class> class Cmp, class Tuple>
using sort_types_t = reorder_types_t<Tuple, sort_types_indices_t<Cmp, Tuple>>;


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


template<class Tuple>
using types_sizeof_optimization_t = sort_types_t<greater_eq_sizeof, Tuple>;


template<class Tuple, class T>
using types_unique_push_back_t = types_insert_back_if_t<Tuple, !types_has_type_v<T, Tuple>, T>;


namespace private_detail_types_algorithm
{
    namespace private_detail_types_unique_insert_back
    {
        template<class Tuple, class... Types>
        struct types_unique_insert_back;

        template<class Tuple>
        struct types_unique_insert_back<Tuple>
        {
            using type = Tuple;
        };

        template<class Tuple, class T0, class... Types>
        struct types_unique_insert_back<Tuple, T0, Types...>
        {
            using type = typename types_unique_insert_back<
                types_unique_push_back_t<Tuple, T0>,
                Types...
            >::type;
        };

        template<class Tuple, class... Types>
        using types_unique_insert_back_t = typename types_unique_insert_back<Tuple, Types...>::type;
    }
}

using private_detail_types_algorithm::private_detail_types_unique_insert_back::types_unique_insert_back_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_types_unique_push_back_pack
    {
        template<template <class...> class Tuple, class Pack, class TailPack>
        struct types_unique_push_back_pack;

        template<template <class...> class Tuple, class... Types, class... Tail>
        struct types_unique_push_back_pack<Tuple, Tuple<Types...>, Tuple<Tail...> >
        {
            using type = types_unique_insert_back_t<Tuple<Types...>, Tail...>;
        };

        template<template <class...> class Tuple, class Pack, class TailPack>
        using types_unique_push_back_pack_t = typename types_unique_push_back_pack<Tuple, Pack, TailPack>::type;
    }
}

using private_detail_types_algorithm::private_detail_types_unique_push_back_pack::types_unique_push_back_pack_t;


namespace private_detail_types_algorithm
{
    namespace private_detail_types_unique
    {
        template<class Tuple>
        struct types_unique;

        template<template <class...> class Tuple, class... Types>
        struct types_unique<Tuple<Types...>>
        {
            using type = types_unique_insert_back_t<Tuple<>, Types...>;
        };

        template<class Tuple>
        using types_unique_t = typename types_unique<Tuple>::type;
    }
}

using private_detail_types_algorithm::private_detail_types_unique::types_unique_t;