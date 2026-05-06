#pragma once

#include <core/vec2.h>
#include <core/view.h>


template<class T>
struct size2d : vec2<T>
{
    using vec2_type = vec2<T>;
    using typename vec2_type::value_type;
    using typename vec2_type::reference;
    using typename vec2_type::const_reference;
    using vec2_type::_0;
    using vec2_type::_1;

    [[nodiscard]]
    constexpr value_type width() const noexcept
    {
        return cref_width();
    }

    [[nodiscard]]
    constexpr value_type height() const noexcept
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
    constexpr size2d with_width(value_type value) const noexcept
    {
        return { std::move(value), _1 };
    }

    [[nodiscard]]
    constexpr size2d with_height(value_type value) const noexcept
    {
        return { _0, std::move(value) };
    }

    [[nodiscard]] constexpr bool has_positiven_mark() const noexcept
    {
        const auto result = ::is_positiven(_1);
        D_ASSERT(result == ::is_positiven(_0));
        return result;
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
[[nodiscard]] constexpr auto to_size2d(const T& v) noexcept -> size2d<decltype(as_vec2(v)._0)>
{
    return { as_vec2(v) };
}

namespace private_detail_sizes
{
    using namespace ordered_overload;

    template<class T>
    using decl_size2d_t = std::remove_reference_t<decltype(as_size2d(std::declval<const T&>()))>;

    template<class T>
    [[nodiscard]] constexpr decl_size2d_t<T> sizes1(const T& value, _order<_2>) noexcept
    {
        return value;
    }

    template<class T>
    [[nodiscard]] constexpr auto sizes1(const T& value, _order<_1>) noexcept -> decl_size2d_t<decltype(value.sizes())>
    {
        return value.sizes();
    }

    template<class T>
    [[nodiscard]] constexpr auto sizes1(const T& value, _order<_0>) noexcept -> decl_size2d_t<decltype(value.sizes)>
    {
        return value.sizes;
    }

    template<class T>
    [[nodiscard]] constexpr auto sizes1(const T& value) noexcept -> decltype(sizes1(value, _start))
    {
        return sizes1(value, _start);
    }

    template<class T>
    [[nodiscard]] constexpr auto sizes0(const T& value, _order<_2>) noexcept -> decltype(sizes1(view(value)))
    {
        return sizes1(view(value));
    }

    template<class T>
    [[nodiscard]] constexpr auto sizes0(const T& value, _order<_1>) noexcept -> decltype(sizes1(value.r()))
    {
        return sizes1(value.r());
    }

    template<class T>
    [[nodiscard]] constexpr auto sizes0(const T& value, _order<_0>) noexcept -> decltype(sizes1(value))
    {
        return sizes1(value);
    }

    template<class T>
    [[nodiscard]] constexpr auto sizes(const T& value) noexcept -> decltype(sizes0(value, _start))
    {
        return sizes0(value, _start);
    }

    template<class T>
    [[nodiscard]] constexpr auto width0(const T& value, _order<_1>) noexcept -> decltype(sizes(value).width())
    {
        return sizes(value).width();
    }

    template<class T>
    [[nodiscard]] constexpr auto height0(const T& value, _order<_1>) noexcept -> decltype(sizes(value).height())
    {
        return sizes(value).height();
    }

    template<class T>
    [[nodiscard]] constexpr auto width0(const T& value, _order<_0>) noexcept -> decltype(value.width())
    {
        return value.width();
    }

    template<class T>
    [[nodiscard]] constexpr auto height0(const T& value, _order<_0>) noexcept -> decltype(value.height())
    {
        return value.height();
    }

    template<class T>
    [[nodiscard]] constexpr auto width(const T& value) noexcept -> decltype(width0(value, _start))
    {
        return width0(value, _start);
    }

    template<class T>
    [[nodiscard]] constexpr auto height(const T& value) noexcept -> decltype(height0(value, _start))
    {
        return height0(value, _start);
    }
}

using private_detail_sizes::sizes;
using private_detail_sizes::width;
using private_detail_sizes::height;


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