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
    struct value_type_detector1
    {};

    template <class C>
    struct value_type_detector1<C, std::void_t<decl_data_pointer_t<C>>>
    {
        using type = std::remove_pointer_t<decl_data_pointer_t<C>>;
    };

    template <class C, class = void>
    struct value_type_detector0 : value_type_detector1<C>
    {};

    template <class C>
    struct value_type_detector0<C, std::void_t<decl_value_type_t<C>>>
    {
        using type = decl_value_type_t<C>;
    };

    template <class C>
    struct value_type_type : value_type_detector0<C>
    {};

    template<class C>
    using value_type_t = typename value_type_type<C>::type;
}

using private_detail_value_type::decl_data_pointer_t;
using private_detail_value_type::decl_value_type_t;
using private_detail_value_type::value_type_type;
using private_detail_value_type::value_type_t;