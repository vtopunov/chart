#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>

#include <core/warnings.h>


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

#define D_DEFAULT_COPY_MOVE(Class) \
    D_DEFAULT_COPY(Class); \
    D_DEFAULT_MOVE(Class)   

#define D_DEFAULT_ALL_CA(Class) \
    constexpr Class() noexcept = default;\
    D_DEFAULT_COPY_MOVE(Class)


#define D_UNUSED(expression) ((void)(expression))


#ifdef NDEBUG
#define D_IS_DEBUG 0
#define D_ONLY_DEBUG(A)
#define D_ASSERT(expression) D_UNUSED(0)
#define D_ASSERT_OR_UNUSED(expression) D_UNUSED(expression)


#else
#define D_IS_DEBUG 1
#define D_ONLY_DEBUG(A) A

#ifdef _MSC_VER
#define D_ASSERT(expression) D_UNUSED((!!(expression)) || ((__debugbreak()), 0))
#else
#define D_ASSERT(expression) assert(expression)
#endif

#define D_ASSERT_OR_UNUSED(expression) D_ASSERT(expression)


#endif


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
struct basic_intrusive_node;

template<class T>
struct intrusive_node_object;

template<class T>
class intrusive_list_view;

template<class T>
class intrusive_list;


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


template<class T>
class optional_reference_wrapper;


template <class T, class D>
class unique_resource;

template<class T, class D>
class shared_resource;


struct nothing
{
    template<class... Args>
    constexpr void operator () (Args&&...) const noexcept
    {}
};