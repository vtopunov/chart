#pragma once

#include <core/buffer_view.h>
#include <core/span.h>


namespace private_detail_view
{
    template<class T>
    using is_small_size = std::bool_constant<sizeof(T) <= small_size_mini>;

    template<class T>
    constexpr auto is_view_by_copy_v = std::disjunction_v<
        std::is_scalar<T>,
        std::conjunction<std::is_trivially_copyable<T>, is_small_size<T>>
    >;

    template<class T>
    using view_by_copy0_t = std::conditional_t<is_view_by_copy_v<T>, T, std::add_lvalue_reference_t<std::add_const_t<T>>>;

    template<class T, class = void>
    struct view_type1
    {
        using type = view_by_copy0_t<T>;
    };

    template<class T>
    struct view_type1<T, std::void_t<value_type_t<T>>>
    {
        using value_type = value_type_t<T>;
        using type = std::conditional_t<
            std::is_void_v<value_type>, const_byte_buffer_view, span<std::add_const_t<value_type>, extent_v<T>>
        >;
    };

    template<class T, class = void>
    struct view_type0 : view_type1<T>
    {};

    template<class T>
    struct view_type0<T, std::void_t<decl_view_type_t<T>>>
    {
        using type = decl_view_type_t<T>;
    };

    template <class T>
    using view_t = std::add_const_t<typename view_type0<std::remove_cvref_t<T>>::type>;


    template<class T, class = void>
    struct cview_type0
    {
        using type = view_t<view_type0<T>>;
    };

    template<class T>
    struct cview_type0<T, std::void_t<decl_const_view_type_t<T>>>
    {
        using type = decl_const_view_type_t<T>;
    };

    template <class T>
    using cview_t = std::add_const_t<typename cview_type0<std::remove_cvref_t<T>>::type>;

    template<class T>
    using view_by_copy_t = std::add_const_t<view_by_copy0_t<std::remove_cvref_t<T>>>;
}

using private_detail_view::view_t;
using private_detail_view::cview_t;
using private_detail_view::view_by_copy_t;
using private_detail_view::is_view_by_copy_v;


template<class T> [[nodiscard]]
constexpr view_t<T> view(const T& r) noexcept
{
    return static_cast<view_t<T>>(r);
}

template<class T> [[nodiscard]]
constexpr auto cview(const T& r) noexcept -> decltype(static_cast<cview_t<T>>(r))
{
    return static_cast<cview_t<T>>(r);
}