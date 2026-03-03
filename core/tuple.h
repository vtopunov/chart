#pragma once

#include <core/utility.h>


template <>
struct tuple<>
{
    D_DEFAULT_EQ_OP(tuple);
};

template <class T0>
struct tuple<T0>
{
    D_NO_UNIQUE_ADDRESS T0 _0;

    D_DEFAULT_EQ_OP(tuple);
};

template <class T0, class T1>
struct tuple<T0, T1>
{
    D_NO_UNIQUE_ADDRESS T0 _0;
    D_NO_UNIQUE_ADDRESS T1 _1;

    D_DEFAULT_EQ_OP(tuple);
};

template <class T0, class T1, class T2>
struct tuple<T0, T1, T2>
{
    D_NO_UNIQUE_ADDRESS T0 _0;
    D_NO_UNIQUE_ADDRESS T1 _1;
    D_NO_UNIQUE_ADDRESS T2 _2;

    D_DEFAULT_EQ_OP(tuple);
};

template <class T0, class T1, class T2, class T3>
struct tuple<T0, T1, T2, T3>
{
    D_NO_UNIQUE_ADDRESS T0 _0;
    D_NO_UNIQUE_ADDRESS T1 _1;
    D_NO_UNIQUE_ADDRESS T2 _2;
    D_NO_UNIQUE_ADDRESS T3 _3;

    D_DEFAULT_EQ_OP(tuple);
};

template <class T0, class T1, class T2, class T3, class T4>
struct tuple<T0, T1, T2, T3, T4>
{
    D_NO_UNIQUE_ADDRESS T0 _0;
    D_NO_UNIQUE_ADDRESS T1 _1;
    D_NO_UNIQUE_ADDRESS T2 _2;
    D_NO_UNIQUE_ADDRESS T3 _3;
    D_NO_UNIQUE_ADDRESS T4 _4;

    D_DEFAULT_EQ_OP(tuple);
};


namespace private_detail_tuple
{
    template<class... Types>
    struct tuple_map : Types...
    {
        using Types::_get_ptr_value_impl...;

        D_DEFAULT_EQ_OP(tuple_map);
    };

    template<size_t Index, class T>
    struct tuple_map_element
    {
        D_NO_UNIQUE_ADDRESS T value;

        [[nodiscard]]
        constexpr const T* _get_ptr_value_impl(index_constant<Index>) const noexcept
        {
            return std::addressof(value);
        }

        [[nodiscard]]
        constexpr T* _get_ptr_value_impl(index_constant<Index>) noexcept
        {
            return std::addressof(value);
        }

        D_DEFAULT_EQ_OP(tuple_map_element);
    };

    template <class IS, class... Types>
    struct make_tuple_map_helper;

    template <size_t... Indices, class... Types>
    struct make_tuple_map_helper<std::index_sequence<Indices...>, Types...>
    {
        using type = tuple_map<tuple_map_element<Indices, Types>...>;
    };

    template<class... Types>
    using make_tuple_map_t = typename make_tuple_map_helper<std::make_index_sequence<sizeof...(Types)>, Types...>::type;

    template<class S, class T>
    using ptr_t = copy_const_t<S, T>*;

    template<class V>
    [[nodiscard]] constexpr auto get_ptr_impl(index_constant<0>, V* p) noexcept -> ptr_t<V, decltype(V::_0)>
    {
        return std::addressof(p->_0);
    }

    template<class V>
    [[nodiscard]] constexpr auto get_ptr_impl(index_constant<1>, V* p) noexcept -> ptr_t<V, decltype(V::_1)>
    {
        return std::addressof(p->_1);
    }

    template<class V>
    [[nodiscard]] constexpr auto get_ptr_impl(index_constant<2>, V* p) noexcept -> ptr_t<V, decltype(V::_2)>
    {
        return std::addressof(p->_2);
    }

    template<class V>
    [[nodiscard]] constexpr auto get_ptr_impl(index_constant<3>, V* p) noexcept -> ptr_t<V, decltype(V::_3)>
    {
        return std::addressof(p->_3);
    }

    template<class V>
    [[nodiscard]] constexpr auto get_ptr_impl(index_constant<4>, V* p) noexcept -> ptr_t<V, decltype(V::_4)>
    {
        return std::addressof(p->_4);
    }

    template<class Index, class V>
    [[nodiscard]] constexpr auto get_ptr_impl(Index index, V* p) noexcept -> decltype(p->_get_ptr_value_impl(index))
    {
        return p->_get_ptr_value_impl(index);
    }

    template<size_t i, class V>
    [[nodiscard]] constexpr auto get_ptr(V* p) noexcept -> decltype(get_ptr_impl(index_constant_v<i>, p))
    {
        return get_ptr_impl(index_constant_v<i>, p);
    }
}

using private_detail_tuple::get_ptr;

template <class T0, class T1, class T2, class T3, class T4, class... Types>
struct tuple<T0, T1, T2, T3, T4, Types...> : private_detail_tuple::make_tuple_map_t<T0, T1, T2, T3, T4, Types...>
{};

template<size_t i, class V>
[[nodiscard]] constexpr auto get(V& v) -> std::add_lvalue_reference_t<std::remove_pointer_t<decltype(get_ptr<i>(std::addressof(v)))> >
{
    return *get_ptr<i>(std::addressof(v));
}

template<class... Types>
[[nodiscard]] constexpr tuple<Types...>& as_tuple(tuple<Types...>& tuple_ref) noexcept
{
    return tuple_ref;
}
template<class... Types>
[[nodiscard]] constexpr const tuple<Types...>& as_tuple(const tuple<Types...>& tuple_ref) noexcept
{
    return tuple_ref;
}

