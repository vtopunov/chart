#pragma once

#include <core/fwd.h>
#include <core/type_traits.h>


namespace private_detail_view
{
    constexpr auto small_size_v = 4u * sizeof(size_t);
    static_assert(small_size_v >= 2u * sizeof(size_t));

    template<class T>
    using is_small_size = std::bool_constant<sizeof(T) <= small_size_v>;

    template<class T>
    using is_view_by_copy = std::disjunction
    <
        std::is_scalar<T>, 
        std::conjunction<std::is_trivially_copyable<T>, is_small_size<T>>
    >;

    template<class T>
    using view_by_copy_t = std::conditional_t
    <
        is_view_by_copy<T>::value, 
        std::add_const_t<T>, 
        std::add_lvalue_reference_t<std::add_const_t<T>>
    >;

    template<class T>
    using decl_view_t = std::add_const_t<typename T::view_type>;

    template <class T>
    using view_t = detected_or_t<view_by_copy_t<T>, decl_view_t, T>;
}

template<class T>
using view_by_copy_t = private_detail_view::view_by_copy_t<std::remove_cvref_t<T>>;

template <class T>
using view_t = private_detail_view::view_t<std::remove_cvref_t<T>>;

template<class T> [[nodiscard]]
constexpr view_t<T> view(const T& r) noexcept
{
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_do_not_slice);
    return static_cast<view_t<T>>(r);
    D_WARNING_POP;
}

