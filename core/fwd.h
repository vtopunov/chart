#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cerrno>

#include <core/warnings.h>


#define D_DISABLE_COPY_CA(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete

#define D_DISABLE_MOVE_CA(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete

#define D_DEFAULT_COPY_CA(Class) \
    constexpr Class(const Class &) noexcept = default;\
    constexpr Class &operator=(const Class &) noexcept = default

#define D_DEFAULT_MOVE_CA(Class) \
    constexpr Class(Class &&) noexcept = default; \
    constexpr Class &operator=(Class &&) noexcept = default

#define D_DISABLE_COPYMOVE_CA(Class) \
    D_DISABLE_COPY_CA(Class); \
    D_DISABLE_MOVE_CA(Class)

#define D_DEFAULT_ONLYMOVE_CA(Class) \
    D_DISABLE_COPY_CA(Class); \
    D_DEFAULT_MOVE_CA(Class)

#define D_DEFAULT_COPYMOVE_CA(Class) \
    D_DEFAULT_COPY_CA(Class); \
    D_DEFAULT_MOVE_CA(Class)   

#define D_DEFAULT_ALL_CA(Class) \
    constexpr Class() noexcept = default;\
    D_DEFAULT_COPYMOVE_CA(Class)

#define D_DEFAULT_EQ_OP(Class) \
    [[nodiscard]] constexpr bool operator == (const Class&) const noexcept = default; \
    [[nodiscard]] constexpr bool operator != (const Class&) const noexcept = default

#define D_DEFAULT_ALL_CAEQ(Class) \
    D_DEFAULT_ALL_CA(Class); \
    D_DEFAULT_EQ_OP(Class)


#define D_UNUSED(expression) ((void)(expression))

#ifdef NDEBUG
#define D_IS_DEBUG 0
#define D_ONLY_DEBUG(A)
#define D_DEBUG_OR(D, R) R 
#define D_ASSERT(expression) D_UNUSED(0)
#define D_ASSERT_OR_UNUSED(expression) D_UNUSED(expression)


#else
#define D_IS_DEBUG 1
#define D_ONLY_DEBUG(A) A
#define D_DEBUG_OR(D, R) D 

#ifdef _MSC_VER
#define D_ASSERT(expression) D_UNUSED((!!(expression)) || ((__debugbreak()), false))
#else
#define D_ASSERT(expression) assert(expression)
#endif

#define D_ASSERT_OR_UNUSED(expression) D_ASSERT(expression)

#endif


#ifdef _MSC_VER
#define D_FORCEINLINE inline __forceinline
#define D_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]] 
#else
#define D_FORCEINLINE inline __attribute__((always_inline))
#define D_NO_UNIQUE_ADDRESS [[no_unique_address]] 
#endif


using doublemax_t = long double;


constexpr size_t operator "" _uz(unsigned long long value) noexcept
{
    return value;
}

constexpr ptrdiff_t operator "" _z(unsigned long long value) noexcept
{
    return value;
}


constexpr size_t size_maxi{ SIZE_MAX };

constexpr size_t small_size_mini{ 4_uz * sizeof(size_t) };

constexpr size_t dynamic_extent{ size_maxi };


struct memory_construct_t
{
    static constexpr struct construct_tag_t {} tag;

    constexpr explicit memory_construct_t(construct_tag_t) noexcept {}
};

constexpr memory_construct_t memory_construct{ memory_construct_t::tag };

struct nullmem_t
{
    constexpr explicit nullmem_t(memory_construct_t) noexcept
    {}
};

constexpr nullmem_t nullmem{ memory_construct };


template<class T>
struct basic_intrusive_node;

template<class T>
struct intrusive_node_object;

template<class T>
class intrusive_list_ref;

template<class T>
class intrusive_list;


template<class T>
struct rational;


template<class T>
struct vec2;

template<class T>
struct point2d;

template<class T>
struct size2d;

template <class T, size_t = dynamic_extent>
class span;

template<class T>
class buffer;

static_assert(1_uz == sizeof(std::byte));
using byte_buffer = buffer<std::byte>;

template<bool immutable>
class basic_buffer_view;

using buffer_view = basic_buffer_view<false>;
using const_buffer_view = basic_buffer_view<true>;

template<class T>
class optional_reference_wrapper;


constexpr auto rgba_color_extent = 4_uz;

using argb_t = uint32_t;
static_assert(sizeof(argb_t) == rgba_color_extent);

using luminance_t = uint8_t;
static_assert(sizeof(luminance_t) == 1_uz);

using luminancef_t = float;
static_assert(sizeof(luminancef_t) == 4_uz);

template<class T>
using basic_rgba_color_view = span<const T, rgba_color_extent>;

template<class T>
struct basic_rgba_color;

using rgba_color_view = basic_rgba_color_view<luminance_t>;
using rgbaf_color_view = basic_rgba_color_view<luminancef_t>;
using rgba_color = basic_rgba_color<luminance_t>;
using rgbaf_color = basic_rgba_color<luminancef_t>;


template<class C>
using decl_value_type_t = typename C::value_type;

template<class T>
using decl_view_type_t = typename T::view_type;

template<class T>
using decl_null_type_t = typename T::null_type;


template <class T, class D>
class unique_resource;

template<class T, class D>
class shared_resource;


struct nothing
{
    template<class... Args>
    constexpr void operator () (const Args&...) const noexcept
    {}
};

struct dummy {};

template<class...>
struct types_pack
{};

template<class... Types>
constexpr types_pack<Types...> types_pack_v{};

template<template <class...> class...>
struct tuples_pack
{};

template<template <class...> class Tuple>
struct tuple_pack
{};

using noapply_t = types_pack<>;

constexpr noapply_t noapply{};
