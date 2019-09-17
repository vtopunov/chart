#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <limits>
#include <numeric>
#include <compare>
#include <array>
#include <utility>

#include <core/narrow_cast.h>
#include <core/assert.h>

#undef min
#undef max

using real_t = double;

using byte_t = unsigned char;

using int_t = ptrdiff_t;

template<class T>
constexpr T max_v = std::numeric_limits<T>::max();

template<class T>
constexpr T lowest_v = std::numeric_limits<T>::lowest();

template<class To, class From>
To bit_cast(From from) noexcept // c++20
{
    constexpr size_t size = std::min(sizeof(To), sizeof(From));
    std::remove_const_t<To> to{};
    std::memcpy(&to, &from, size);
    return to;
}

constexpr std::size_t operator "" _z(unsigned long long n) noexcept // c++20
{
    return n;
}

constexpr unsigned long long operator "" _Kb(unsigned long long n) noexcept
{
    return n * 1024ULL;
}

constexpr unsigned long long operator "" _Mb(unsigned long long n) noexcept
{
    return n * 1024_Kb;
}

template<class T>
constexpr std::make_unsigned_t<T> to_unsingned( T signed_value ) noexcept
{
    return narrow_cast<std::make_unsigned_t<T>>( signed_value );
}

template<class T>
constexpr std::make_signed_t<T> to_singned( T unsigned_value ) noexcept
{
    return narrow_cast<std::make_signed_t<T>>( unsigned_value );
}

constexpr bool is_digit( char c ) noexcept
{
    return c >= '0' && c <= '9';
}

template<class T>
constexpr const T* as_const_pointer( T* pointer ) noexcept
{
    return pointer;
}

template<class T>
constexpr T* as_mutable_pointer( const T* pointer ) noexcept
{
    return const_cast<T*>( pointer );
}