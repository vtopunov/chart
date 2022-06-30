#pragma once

#include <utility>

#include <core/warnings.h>

#ifdef _MSC_VER
#define D_FORCE_INLINE inline __forceinline

#else
#define D_FORCE_INLINE inline __attribute__((always_inline))
#define D_LIKELY(expr)    __builtin_expect(!!(expr), 1L)
#define D_UNLIKELY(expr)  __builtin_expect(!!(expr), 0L)

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

template <class T>
void as_mutable(const T&&) = delete;

D_WARNING_POP


template<class T> [[nodiscard]] 
constexpr bool is_null_or_empty(const T* string) noexcept
{
    return !string || !*string;
}