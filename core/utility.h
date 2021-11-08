#pragma once

#include <type_traits>
#include <utility>
#include <limits>

#include <core/assert.h>

#define D_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete

#define D_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete

#define D_DEFAULT_COPY(Class) \
    Class(const Class &) = default;\
    Class &operator=(const Class &) = default

#define D_DEFAULT_MOVE(Class) \
    Class(Class &&) = default; \
    Class &operator=(Class &&) = default

#define D_DISABLE_COPY_MOVE(Class) \
    D_DISABLE_COPY(Class); \
    D_DISABLE_MOVE(Class)

#define D_DEFAULT_MOVABLE_ONLY(Class) \
    D_DISABLE_COPY(Class); \
    D_DEFAULT_MOVE(Class)


namespace ordered_overload
{
    struct _3
    {};

    struct _2 : _3
    {};

    struct _1 : _2
    {};

    struct _0 : _1
    {};

    template<class T>
    using _overload = const T* const;

    template<class T>
    using _order = _overload<T>;

    template<class T>
    inline constexpr _overload<T> _overload_v{ nullptr };

    inline constexpr _order<_0> _start{ nullptr };
}

template<class T> [[nodiscard]]
constexpr T constexpr_abs(T value) noexcept
{
    if constexpr (std::is_unsigned_v<T>)
    {
        return value;
    }
    else
    {
        constexpr T zero{};
        return ( value < zero ) ? -value : value;
    }
}

template<class T>
inline constexpr auto numeric_max_v = std::numeric_limits<T>::max();

template<class T>
inline constexpr auto numeric_min_v = std::numeric_limits<T>::min();

template<class T>
inline constexpr auto numeric_lowest_v = std::numeric_limits<T>::lowest();

template<class T>
inline constexpr auto numeric_nan_v = std::numeric_limits<T>::quiet_NaN();

template<class T>
inline constexpr auto numeric_inf_v = std::numeric_limits<T>::infinity();


template<bool test, class T>
using add_const_if_t = std::conditional_t<test, std::add_const_t<T>, T>;

template<class S, class D>
using copy_const_t = add_const_if_t<std::is_const_v<S>, D>;

template<class Value, class Old, class New>
using replace_t = std::conditional_t<std::is_same_v<Value, Old>, New, Value>;


template<bool Test, class T>
struct make_unsigned_or0
{
    using type = std::make_unsigned_t<T>;
};

template<class T>
struct make_unsigned_or0<false, T>
{
    using type = T;
};

template<class T>
struct make_unsigned_or : make_unsigned_or0<std::is_integral_v<T>, T>
{};

template<class T>
using make_unsigned_or_t = typename make_unsigned_or<T>::type;


template<class T>
struct add_immutable
{
    using type = std::add_const_t<T>;
};

template<class T>
struct add_immutable<T*>
{
    using type = const T* const;
};

template<class T>
struct add_immutable<T&>
{
    using type = const T&;
};

template<class T>
struct add_immutable<T*&>
{
    using type = const T* const&;
};

template<class T>
struct add_immutable<const T&> : add_immutable<T&>
{};

template<class T>
struct add_immutable<const T> : add_immutable<T>
{};

template<class T>
using add_immutable_t = typename add_immutable<T>::type;


template<class T> [[nodiscard]]
constexpr T* as_pointer(T* ptr) noexcept
{
    return ptr;
}


#pragma warning(push)
#pragma warning(disable : 26465) // Don't use const_cast to cast away const
#pragma warning(disable : 26493) // Don't use C-style casts

template <class T> [[nodiscard]]
constexpr T& as_mutable(const T& value) noexcept
{
    return const_cast<T&>(value);
}

template <class T> [[nodiscard]]
constexpr T* as_mutable(const T* value) noexcept
{
    return const_cast<T*>(value);
}

template <class T> [[nodiscard]]
void as_mutable(const T&&) = delete;

#pragma warning(pop)


template <class T> [[nodiscard]]
constexpr add_immutable_t<T&> as_immutable(T& value) noexcept
{
    return value;
}

template <class T> [[nodiscard]]
constexpr add_immutable_t<T*> as_immutable(T* value) noexcept
{
    return value;
}

template <class T>
void as_immutable(const T&&) = delete;

#pragma warning(push)
#pragma warning(disable : 26472) //  Don't use a static_cast for arithmetic conversions. Use brace initialization, narrow_cast or narrow


template<class Target, class Source>
constexpr bool is_unsigned2_v = std::is_unsigned_v<Source> && std::is_unsigned_v<Target>;

template<class word, class dword> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<word, dword>, word> hi_cast(dword dw) noexcept
{
    if constexpr (sizeof(dword) > sizeof(word))
    {
        constexpr auto nbits = 8u * (sizeof(dword) - sizeof(word));

        return static_cast<word>(dw >> nbits);
    }
    else
    {
        return 0u;
    }
}

template<class word, class dword> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<word, dword>, word> lo_cast(dword dw) noexcept
{
    if constexpr (sizeof(dword) > sizeof(word))
    {
        constexpr auto mask = static_cast<dword>(word(-1));

        return static_cast<word>(dw & mask);
    }
    else
    {
        return static_cast<word>(dw);
    }
}

template<class Target, class Source> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<Target, Source>, Target> clamp_cast(Source v) noexcept
{
    if constexpr (sizeof(Source) > sizeof(Target))
    {
        constexpr Source target_max{ numeric_max_v<Target> };

        return (v > target_max) ? target_max : static_cast<Target>(v);
    }
    else
    {
        return static_cast<Target>(v);
    }
}
#pragma warning(pop)

template<size_t mul> [[nodiscard]]
constexpr size_t size_mul(size_t size) noexcept
{
    [[maybe_unused]]
    constexpr size_t overflow = numeric_max_v<size_t> / mul;

    D_ASSERT(size <= overflow);

    return size * mul;
}