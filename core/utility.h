#pragma once

#include <utility>

#include <core/warnings.h>
#include <core/ordered_overload.h>


#ifdef _MSC_VER
#define D_FORCEINLINE inline __forceinline
#define D_LIKELY(expr) expr   
#define D_UNLIKELY(expr) expr
#define D_ATTRIB_LIKELY [[likely]]
#define D_ATTRIB_UNLIKELY [[unlikely]]

#else
#define D_FORCEINLINE inline __attribute__((always_inline))
#define D_LIKELY(expr)    __builtin_expect(!!(expr), 1L)
#define D_UNLIKELY(expr)  __builtin_expect(!!(expr), 0L)
#define D_ATTRIB_LIKELY 
#define D_ATTRIB_UNLIKELY

#endif

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



template<class T> [[nodiscard]]
constexpr T* as_pointer(T* ptr) noexcept
{
    return ptr;
}

template<class T> [[nodiscard]]
constexpr const T*const as_const_pointer(const T* ptr) noexcept
{
    return ptr;
}

template<class T> [[nodiscard]]
constexpr T* as_mutable_pointer(const T* ptr) noexcept
{
    return const_cast<T*>(ptr);
}

template <class T> [[nodiscard]]
constexpr T& as_reference(T& value) noexcept
{
    return value;
}


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast)

template <class T> [[nodiscard]]
constexpr T& as_mutable(const T& value) noexcept
{
    return const_cast<T&>(value);
}

D_WARNING_POP


template<class T> [[nodiscard]] 
constexpr bool is_null_or_empty(const T* string) noexcept
{
    return !string || !*string;
}


template<class T, class U>
constexpr T& min_eq(T& value, U&& new_value) noexcept
{
    if (new_value < value)
    {
        value = std::forward<U>(new_value);
    }

    return value;
}

template<class T, class U>
constexpr T& max_eq(T& value, U&& new_value) noexcept
{
    if (value < new_value)
    {
        value = std::forward<U>(new_value);
    }

    return value;
}


template<class Fn, class Arg, class = void>
struct function_filter
{
    Fn fn;
};

template <class Fn, class Arg>
struct function_filter<Fn, Arg, std::void_t<decltype(std::declval<Fn&>()(std::declval<Arg>()))>>
{
    Fn fn;

    decltype(auto) operator () (Arg arg) noexcept
    {
        return fn(std::move(arg));
    }
};


namespace private_detail_swap
{
    using namespace ordered_overload;

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, _order<_3>) noexcept -> decltype(as_reference((std::swap<R>(right, left), right)))
    {
        std::swap<R>(right, left);
        return right;
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, _order<_2>) noexcept -> decltype(as_reference((std::swap<L>(left, right), left)))
    {
        std::swap<L>(left, right);
        return left;
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, _order<_1>) noexcept -> decltype(as_reference((right.swap(left), right)))
    {
        right.swap(left);
        return right;
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, _order<_0>) noexcept -> decltype(as_reference((left.swap(right), left)))
    {
        left.swap(right);
        return left;
    }

    template<class L, class R>
    constexpr auto swap(L& left, R& right) noexcept -> decltype(swap_impl(left, right, _start))
    {
        return swap_impl(left, right, _start);
    }
}

using private_detail_swap::swap;