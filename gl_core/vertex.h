#pragma once

#include <core/type_traits.h>


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

    namespace private_detail_vertex_get_ptr
    {
        template<size_t i>
        struct index
        {};

        template<size_t i>
        constexpr index<i> index_v{};

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

        template<size_t i, class V>
        constexpr auto get_ptr(V* p) noexcept -> decltype(get_ptr_impl(index_v<i>, p))
        {
            return get_ptr_impl(index_v<i>, p);
        }
    }

    using private_detail_vertex_get_ptr::get_ptr;
}

