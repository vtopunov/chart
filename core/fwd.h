#pragma once

#include <cstddef>
#include <cstdint>

#include <core/warnings.h>
#include <core/ordered_overload.h>


#ifdef _MSC_VER
#define D_FORCEINLINE inline __forceinline
#else
#define D_FORCEINLINE inline __attribute__((always_inline))
#endif


constexpr size_t operator "" _uz(unsigned long long value) noexcept
{
    return value;
}

constexpr ptrdiff_t operator "" _z(unsigned long long value) noexcept
{
    return value;
}


template<class T>
struct vec2;

template<class T>
struct point2d;

template<class T>
struct size2d;


using u32argb_t = uint32_t;
static_assert(sizeof(u32argb_t) == 4u);

using u8tint_t = uint8_t;
static_assert(sizeof(u8tint_t) == 1u);

template<class T>
struct rgba_color;

using rgba_color32_t = rgba_color<u8tint_t>;


using doublemax_t = long double;

template<class T>
struct rational;


template<class T>
class buffer;

static_assert(1u == sizeof(std::byte));
using buffer_t = buffer<std::byte>;


template <class T, class D>
class unique_resource;

template<class T, class D>
class shared_resource;