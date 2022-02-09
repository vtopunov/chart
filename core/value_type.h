#pragma once

#include <iterator>

#include <core/member_detector.h>
#include <core/utility.h>

template<class C>
using decl_data_pointer_t = decltype(as_pointer(std::data(std::declval<C&>())));

template<class T>
using decl_value_type_t = typename T::value_type;

template<class C>
struct decl_value_data_pointer
{
    using type = std::remove_pointer_t<decl_data_pointer_t<C>>;
};

template<class C>
struct decl_value_type
{
    using type = decl_value_type_t<C>;
};

template<class C>
struct value_type_type0
{
    using method_type = std::conditional_t<
        is_detected_v<decl_value_type_t, C>,
        decl_value_type<C>,
        decl_value_data_pointer<C>
    >;

    using type = typename method_type::type;
};

template<class C>
struct value_type_type
{
    using type = typename value_type_type0<std::remove_cvref_t<C>>::type;
};

template<class C>
using value_type_t = typename value_type_type<C>::type;