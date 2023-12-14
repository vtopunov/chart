#pragma once

#include <utility>

#include <core/fwd.h>


#define D_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete

#define D_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete

#define D_DEFAULT_COPY(Class) \
    constexpr Class(const Class &) noexcept = default;\
    constexpr Class &operator=(const Class &) noexcept = default

#define D_DEFAULT_MOVE(Class) \
    constexpr Class(Class &&) noexcept = default; \
    constexpr Class &operator=(Class &&) noexcept = default

#define D_DISABLE_COPY_MOVE(Class) \
    D_DISABLE_COPY(Class); \
    D_DISABLE_MOVE(Class)

#define D_DEFAULT_MOVABLE_ONLY(Class) \
    D_DISABLE_COPY(Class); \
    D_DEFAULT_MOVE(Class)



template<class T>
[[nodiscard]] constexpr T* as_pointer(T* ptr) noexcept
{
    return ptr;
}

template<class T>
[[nodiscard]] constexpr const T* const as_const_pointer(const T* ptr) noexcept
{
    return ptr;
}

template<class T>
[[nodiscard]] constexpr T* as_mutable_pointer(const T* ptr) noexcept
{
    return const_cast<T*>(ptr);
}

template <class T>
[[nodiscard]] constexpr T& as_reference(T& value) noexcept
{
    return value;
}


D_WARNING_PUSH;
D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast);

template <class T>
[[nodiscard]] constexpr T& as_mutable(const T& value) noexcept
{
    return const_cast<T&>(value);
}

D_WARNING_POP;

template<class T>
[[nodiscard]] constexpr bool is_null_or_empty(const T* string) noexcept
{
    return !string || !*string;
}


namespace private_detail_swap
{
    using namespace ordered_overload;

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, std::false_type, _order<_3>) noexcept -> decltype((void)(std::swap<R>(right, left)))
    {
        std::swap<R>(right, left);
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, std::false_type, _order<_2>) noexcept -> decltype((void)(std::swap<L>(left, right)))
    {
        std::swap<L>(left, right);
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, std::false_type, _order<_1>) noexcept -> decltype((void)(right.swap(left)))
    {
        right.swap(left);
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, std::false_type, _order<_0>) noexcept -> decltype((void)(left.swap(right)))
    {
        left.swap(right);
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, std::true_type, _order<_3>) noexcept -> decltype((void)(std::swap<L>(left, right)))
    {
        std::swap<L>(left, right);
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, std::true_type, _order<_2>) noexcept -> decltype((void)(std::swap<R>(right, left)))
    {
        std::swap<R>(right, left);
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, std::true_type, _order<_1>) noexcept -> decltype((void)(left.swap(right)))
    {
        left.swap(right);
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, std::true_type, _order<_0>) noexcept -> decltype((void)(right.swap(left)))
    {
        right.swap(left);
    }

    template<class L, class R>
    constexpr std::disjunction
        <
        std::conjunction<std::is_base_of<L, R>, std::negation<std::is_base_of<R, L>>>,
        std::conjunction<std::is_convertible<R, L>, std::negation<std::is_convertible<L, R>>>
        > R_is_full_declval;

    template<class L, class R>
    constexpr auto swap(L& left, R& right) noexcept -> decltype((void)swap_impl(left, right, R_is_full_declval<L, R>, _start))
    {
        swap_impl(left, right, R_is_full_declval<L, R>, _start);
    }
}

using private_detail_swap::swap;


template<class L, class R>
[[nodiscard]] constexpr auto scalar_min
(
    const L& a, 
    const R& b
) noexcept -> std::remove_reference_t<decltype((b < a), std::declval<std::common_type_t<L, R>>())>
{
    return (b < a) ? b : a;
}


template<class L, class R>
[[nodiscard]] constexpr auto scalar_max
(
    const L& a, 
    const R& b
) noexcept -> std::remove_reference_t<decltype((a < b), std::declval<std::common_type_t<L, R>>())>
{
    return (a < b) ? b : a;
}