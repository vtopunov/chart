#pragma once

#include <core/vec2.h>
#include <core/view.h>


template<class T>
struct size2d : vec2<T>
{
    using vec2_type = vec2<T>;
    using vec2_type::_0;
    using vec2_type::_1;
    using reference = T&;
    using const_reference = const T&;

    [[nodiscard]]
    constexpr T width() const noexcept
    {
        return cref_width();
    }

    [[nodiscard]]
    constexpr T height() const noexcept
    {
        return cref_height();
    }

    [[nodiscard]]
    constexpr reference ref_width() noexcept
    {
        return as_mutable(cref_width());
    }

    [[nodiscard]]
    constexpr reference ref_height() noexcept
    {
        return as_mutable(cref_height());
    }

    [[nodiscard]]
    constexpr const_reference ref_width() const noexcept
    {
        return cref_width();
    }

    [[nodiscard]]
    constexpr const_reference ref_height() const noexcept
    {
        return cref_height();
    }

    [[nodiscard]]
    constexpr const_reference cref_width() const noexcept
    {
        return _0;
    }

    [[nodiscard]]
    constexpr const_reference cref_height() const noexcept
    {
        return _1;
    }

    [[nodiscard]]
    constexpr size2d with_width(T value) const noexcept
    {
        return { std::move(value), _1 };
    }

    [[nodiscard]]
    constexpr size2d with_height(T value) const noexcept
    {
        return { _0, std::move(value) };
    }

    [[nodiscard]]
    constexpr bool operator == (const size2d&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const size2d&) const noexcept = default;
};

template<class T>
size2d(T, T) -> size2d<T>;

template<class T>
size2d(const vec2<T>&) -> size2d<T>;


template<class T>
[[nodiscard]] constexpr const size2d<T>& as_size2d(const size2d<T>& sizes) noexcept
{
    return sizes;
}

template<class T>
[[nodiscard]] constexpr size2d<T>& as_size2d(size2d<T>& sizes) noexcept
{
    return sizes;
}

template<class T>
[[nodiscard]] constexpr size2d<T> to_size2d(const vec2<T>& v) noexcept
{
    return { v };
}

namespace private_detail_sizes
{
    template<class T>
    using type_t = typename T::type;

    template<class Default, class T, class = void>
    struct select_sizes2 : type_t<Default>
    {};

    template<class Default, class T>
    struct select_sizes2<Default, T, std::void_t<decltype(as_size2d(std::declval<const T&>()))> >
    {
        [[nodiscard]] static constexpr auto sizes(const T& value) noexcept
        {
            return as_size2d(value);
        }
    };

    template<class Default, class T, class = void>
    struct select_sizes1 : select_sizes2<Default, T>
    {};

    template<class Default, class T>
    struct select_sizes1<Default, T, std::void_t<decltype(as_size2d(std::declval<const T&>().sizes()))> >
    {
        [[nodiscard]] static constexpr auto sizes(const T& value) noexcept
        {
            return as_size2d(value.sizes());
        }
    };

    template<class Default, class T, class = void>
    struct select_sizes0 : select_sizes1<Default, T>
    {};

    template<class Default, class T>
    struct select_sizes0<Default, T, std::void_t<decltype(as_size2d(std::declval<const T&>().sizes))> >
    {
        [[nodiscard]] static constexpr auto sizes(const T& value) noexcept
        {
            return as_size2d(value.sizes);
        }
    };

    template<class T, class = void>
    struct select_view_sizes1
    {};

    template<class T>
    struct select_view_sizes1<T, std::void_t<decl_view_type_t<T>>>
    {
        using type = select_sizes0<nonesuch, decl_view_type_t<T>>;
    };

    template<class T, class = void>
    struct select_view_sizes0 : select_view_sizes1<T>
    {};

    template<class T>
    struct select_view_sizes0<T, std::void_t<decl_resource_type_t<T>>>
    {
        using type = select_sizes0<select_view_sizes1<T>, decl_resource_type_t<T>>;
    };

    template<class T>
    struct sizes_traits : select_sizes0<select_view_sizes0<T>, T>
    {};

    template<class T>
    [[nodiscard]] constexpr auto sizes(const T& value) noexcept -> decltype(sizes_traits<T>::sizes(value))
    {
        return sizes_traits<T>::sizes(value);
    }
}

using private_detail_sizes::sizes;

template<class T>
[[nodiscard]] constexpr auto width(const T& value) noexcept -> decltype(sizes(value).width())
{
    return sizes(value).width();
}

template<class T>
[[nodiscard]] constexpr auto height(const T& value) noexcept -> decltype(sizes(value).height())
{
    return sizes(value).height();
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator * (const Vec<L>& left, const Vec<R>& right) noexcept -> Vec<decltype(as_size2d(left)._0* as_size2d(right)._0)>
{
    return
    {
        left._0 * right._0,
        left._1 * right._1
    };
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator / (const Vec<L>& left, const Vec<R>& right) noexcept -> Vec<decltype(as_size2d(left)._0 / as_size2d(right)._0)>
{
    return
    {
        left._0 / right._0,
        left._1 / right._1
    };
}