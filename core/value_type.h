#pragma once

#include <iterator>

#include <core/utility.h>

namespace private_detail_value_type
{
    template<class C>
    using decl_data_pointer_t = decltype(as_pointer(std::data(std::declval<C&>())));

    template<class C>
    using decl_value_type_t = typename C::value_type;

    template <class C, class = void>
    struct value_type_by_data_pointer
    {
        using is_data_pointer = std::false_type;
    };

    template <class C>
    struct value_type_by_data_pointer<C, std::void_t<decl_data_pointer_t<C>>>
    {
        using type = std::remove_pointer_t<decl_data_pointer_t<C>>;
        using is_data_pointer = std::true_type;
    };

    template<class C, class = void>
    struct value_type_selector : value_type_by_data_pointer<C>
    {};

    template <class C>
    struct value_type_selector<C, std::void_t<decl_value_type_t<C>>>
    {
        using type = decl_value_type_t<C>;
    };

    template <class C>
    struct value_type_type : value_type_selector<C>
    {};

    template<class C>
    using value_type_t = typename value_type_type<C>::type;

    template<class T>
    using is_data_pointer = typename value_type_by_data_pointer<T>::is_data_pointer;
}

using private_detail_value_type::decl_data_pointer_t;
using private_detail_value_type::decl_value_type_t;
using private_detail_value_type::value_type_type;
using private_detail_value_type::value_type_t;
using private_detail_value_type::is_data_pointer;