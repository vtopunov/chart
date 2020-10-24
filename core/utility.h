#pragma once

#include <type_traits>
#include <utility>

#include <core/assert.h>

#define D_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;

#define D_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define D_DISABLE_COPY_MOVE(Class) \
    D_DISABLE_COPY(Class) \
    D_DISABLE_MOVE(Class)

template<bool test, class T>
using add_const_if_t = std::conditional_t<test, std::add_const_t<T>, T>;

template<class Value, class Old, class New>
using replace_t = std::conditional_t<std::is_same_v<Value, Old>, New, Value>;

template<class T>
struct add_immutable
{
    using type = std::add_const_t<T>;
};

template<class T>
struct add_immutable<T*>
{
    using type = const T*const;
};

template<class T>
struct add_immutable<T&>
{
    using type = const T&;
};

template<class T>
struct add_immutable<T*&>
{
    using type = const T*const&;
};

template<class T>
struct add_immutable<const T&> : add_immutable<T&>
{};

template<class T>
struct add_immutable<const T> : add_immutable<T>
{};

template<class T>
using add_immutable_t = typename add_immutable<T>::type;

template<class T>
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

#pragma warning(pop)

template <class T> [[nodiscard]]
void as_mutable(const T&&) = delete;

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
    if constexpr ( sizeof(dword) > sizeof(word) )
    {
        constexpr auto nbits = 8u * ( sizeof(dword) - sizeof(word) );

        return static_cast<word>( dw >> nbits );
    }
    else
    {
        return 0u;
    }
}

template<class word, class dword> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<word, dword>, word> lo_cast(dword dw) noexcept
{
    if constexpr ( sizeof(dword) > sizeof(word) )
    {
        constexpr auto mask = static_cast<dword>( word(-1) );

        return static_cast<word>( dw & mask );
    }
    else
    {
        return static_cast<word>( dw );
    }
}

template<class Target, class Source> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<Target, Source>, Target> clamp_cast(Source v) noexcept
{
    if constexpr ( sizeof(Source) > sizeof(Target) )
    {
        constexpr Source target_max{ (std::numeric_limits<Target>::max)() };

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
    constexpr auto max_size = std::numeric_limits<size_t>::max();

    [[maybe_unused]]
    constexpr size_t overflow = max_size / mul;

    D_ASSERT(size <= overflow);

    return size * mul;
}
