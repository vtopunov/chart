#pragma once

#include <core/utility.h>

namespace gl
{
    template <class... As>
    struct vertex;

    template <class A0>
    struct vertex<A0>
    {
        A0 a0;
    };

    template <class A0, class A1>
    struct vertex<A0, A1>
    {
        A0 a0;
        A1 a1;
    };

    template <class A0, class A1, class A2>
    struct vertex<A0, A1, A2>
    {
        A0 a0;
        A1 a1;
        A2 a2;
    };

    template <class A0, class A1, class A2, class A3>
    struct vertex<A0, A1, A2, A3>
    {
        A0 a0;
        A1 a1;
        A2 a2;
        A3 a3;
    };

    namespace vertex_get_ptr_detail
    {
        template<size_t i>
        struct index
        {};

        template<class S, class T>
        using ptr_t = copy_const_t<S, T>*;

        template<class V>
        constexpr auto get_ptr_impl(index<0>, V* p) noexcept -> ptr_t<V, decltype(V::a0)>
        {
            return std::addressof(p->a0);
        }

        template<class V>
        constexpr auto get_ptr_impl(index<1>, V* p) noexcept -> ptr_t<V, decltype(V::a1)>
        {
            return std::addressof(p->a1);
        }

        template<class V>
        constexpr auto get_ptr_impl(index<2>, V* p) noexcept -> ptr_t<V, decltype(V::a2)>
        {
            return std::addressof(p->a2);
        }

        template<class V>
        constexpr auto get_ptr_impl(index<3>, V* p) noexcept -> ptr_t<V, decltype(V::a3)>
        {
            return std::addressof(p->a3);
        }

        template<size_t n, class V>
        constexpr auto get_ptr(V* p) noexcept -> decltype(get_ptr_impl(std::declval<index<n>>(), std::declval<V*>()))
        {
            constexpr index<n> index_selector{};
            return get_ptr_impl(index_selector, p);
        }
    }

    using vertex_get_ptr_detail::get_ptr;

    template <class Tuple>
    struct vertex_size : std::integral_constant<size_t, 1u>
    {};

    template <class... Types>
    struct vertex_size<vertex<Types...>> : std::integral_constant<size_t, sizeof...(Types)>
    {};

    template <class Tuple>
    struct vertex_size<const Tuple> : vertex_size<Tuple>
    {};

    template <class T>
    inline constexpr size_t vertex_size_v = vertex_size<T>::value;
}

