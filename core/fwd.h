#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cerrno>

#include <core/warnings.h>


#define D_DISABLE_COPY_CA(Class) \
    Class(const Class &) = delete;\
    void operator=(const Class &) = delete

#define D_DISABLE_MOVE_CA(Class) \
    Class(Class &&) = delete; \
    void operator=(Class &&) = delete

#define D_DEFAULT_COPY_CA(Class) \
    constexpr Class(const Class &) noexcept = default;\
    constexpr Class &operator=(const Class &) noexcept = default

#define D_DEFAULT_MOVE_CA(Class) \
    constexpr Class(Class &&) noexcept = default; \
    constexpr Class &operator=(Class &&) noexcept = default

#define D_DISABLE_COPYMOVE_CA(Class) \
    D_DISABLE_COPY_CA(Class); \
    D_DISABLE_MOVE_CA(Class)

#define D_DISABLE_ALL_CA(Class) \
    constexpr Class() noexcept = delete;\
    D_DISABLE_COPYMOVE_CA(Class)

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

#ifdef _MSC_VER
#define D_FORCEINLINE inline __forceinline
#define D_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#define D_ASSUME(expression) [[assume(expression)]]

#else
#define D_FORCEINLINE inline __attribute__((always_inline))
#define D_NO_UNIQUE_ADDRESS [[no_unique_address]]
#define D_ASSUME(expression) D_UNUSED(0)

#endif


constexpr struct
{
    template<class T>
    constexpr void operator () ([[maybe_unused]] const T& expression_result) const noexcept
    {
        D_ASSUME(expression_result);
    }
} assume_r;


#ifdef NDEBUG
#define D_IS_DEBUG 0
#define D_ONLY_DEBUG(A)
#define D_DEBUG_OR(D, R) R 
#define D_ASSERT(expression) D_ASSUME(expression)
#define D_CHECK(expression) assume_r(expression)

#else
#define D_IS_DEBUG 1
#define D_ONLY_DEBUG(A) A
#define D_DEBUG_OR(D, R) D 

#ifdef _MSC_VER
#define D_ASSERT(expression) D_UNUSED((!!(expression)) || ((__debugbreak()), false))
#else
#define D_ASSERT(expression) assert(expression)
#endif

#define D_CHECK(expression) D_ASSERT(expression)
#endif


constexpr size_t operator ""_uz(unsigned long long value) noexcept
{
    return value;
}

constexpr ptrdiff_t operator ""_z(unsigned long long value) noexcept
{
    return value;
}


template<class R, class... Args>
using function_pointer_t = R(*) (Args...);

template<class R, class... Args>
using noexcept_function_pointer_t = R(*) (Args...) noexcept;


using doublemax_t = long double;
using realf_t = float;
using real_t = double;
using denomi_t = uintmax_t;

constexpr size_t nbyte_arch{ sizeof(void*) };
static_assert(nbyte_arch == sizeof(size_t));
static_assert(nbyte_arch == sizeof(ptrdiff_t));

constexpr denomi_t dynamic_denominator{ 0u };
constexpr size_t dynamic_extent{ SIZE_MAX };
constexpr size_t small_size_mini{ 4u * nbyte_arch };

template<size_t L, size_t R>
constexpr size_t max_size_v = (L < R) ? R : L;


struct uninitialized_t
{
    static constexpr struct {} tag{};
    constexpr explicit uninitialized_t(decltype(tag)) noexcept {}
};

struct memory_construct_t
{
    static constexpr struct {} tag{};
    constexpr explicit memory_construct_t(decltype(tag)) noexcept {}
};

struct memory_overwrite_construct_t
{
    static constexpr struct {} tag{};
    constexpr explicit memory_overwrite_construct_t(decltype(tag)) noexcept {}
};

struct nulltype_construct_t
{
    static constexpr struct {} tag{};
    constexpr explicit nulltype_construct_t(decltype(tag)) noexcept {}
};

struct nullmem_t
{
    constexpr explicit nullmem_t(nulltype_construct_t) noexcept {}
};

struct nullref_t
{
    constexpr explicit nullref_t(nulltype_construct_t) noexcept {}
};

struct nullresource_t
{
    constexpr explicit nullresource_t(nulltype_construct_t) noexcept {}
};

struct nullfunction_t
{
    constexpr explicit nullfunction_t(nulltype_construct_t) noexcept {}
};

constexpr uninitialized_t uninitialized_v{ uninitialized_t::tag };
constexpr memory_construct_t memory_construct{ memory_construct_t::tag };
constexpr memory_overwrite_construct_t memory_overwrite_construct{ memory_overwrite_construct_t::tag };
constexpr nulltype_construct_t nulltype_construct{ nulltype_construct_t::tag };
constexpr nullmem_t nullmem{ nulltype_construct };
constexpr nullref_t nullref{ nulltype_construct };
constexpr nullresource_t nullresource{ nulltype_construct };
constexpr nullfunction_t nullfunction{ nulltype_construct };


template<class T> struct basic_intrusive_node;
template<class T> struct intrusive_node_object;
template<class T> class intrusive_list_ref;
template<class T> class intrusive_list;


template <class T, denomi_t = dynamic_denominator>
struct rational;


template<class... Ts> struct tuple;
template<class T> struct vec2;
template<class T> struct point2d;
template<class T> struct size2d;

template <class T, size_t E = dynamic_extent>
class span;

using vec2re = vec2<real_t>;
using point2re = point2d<real_t>;
using size2re = size2d<real_t>;
using point2re_cspan = span<const point2re>;


template<class T> class basic_buffer_view;
template<class T, size_t A> class basic_buffer;

template<class T, size_t A = max_size_v<alignof(T), nbyte_arch> > 
using buffer = basic_buffer<T, max_size_v<A, nbyte_arch> >;

using byte_buffer_view = basic_buffer_view<std::byte>;
using const_byte_buffer_view = basic_buffer_view<const std::byte>;
using byte_buffer = buffer<std::byte>;
static_assert(1_uz == sizeof(std::byte));
static_assert(nbyte_arch >= alignof(std::byte));


constexpr auto rgba_color_extent = 4_uz;

using argb_t = uint32_t;
using luminance_t = uint8_t;
using luminancef_t = realf_t;
static_assert(sizeof(argb_t) == rgba_color_extent);
static_assert(sizeof(luminance_t) == 1_uz);
static_assert(sizeof(luminancef_t) == 4_uz);

template<class T>
using basic_rgba_color_view = span<const T, rgba_color_extent>;

template<class T>
struct basic_rgba_color;

using rgba_color_view = basic_rgba_color_view<luminance_t>;
using rgbaf_color_view = basic_rgba_color_view<luminancef_t>;
using rgba_color = basic_rgba_color<luminance_t>;
using rgbaf_color = basic_rgba_color<luminancef_t>;


template<class T> using decl_value_type_t = typename T::value_type;
template<class T> using decl_view_type_t = typename T::view_type;
template<class T> using decl_const_view_type_t = typename T::const_view_type;
template<class T> using decl_null_type_t = typename T::null_type;
template<class T> using decl_deleter_type_t = typename T::deleter_type;


template <class T, class D = decl_deleter_type_t<T> >
class unique_resource;

template<class T, class D = decl_deleter_type_t<T> >
class shared_resource;


struct nothing
{
    template<class... Args>
    constexpr void operator () (const Args&...) const noexcept {}
};

template<class...>
struct ttypes {};

template<template <class...> class...>
struct ttuples {};

using dummy = ttypes<>;

struct no_overload
{
    template<class T>
    constexpr no_overload(const T&) noexcept {}
};

template<class T>
struct no_overload_for
{
    constexpr no_overload_for(const T&) noexcept {}
};

D_WARNING_PUSH
D_WARNING_DISABLE_CLANG("-Wundefined-inline")
struct any_overload
{

    template<class T>
    constexpr operator T () const noexcept;
};
D_WARNING_POP

template<class... Types>
constexpr ttypes<Types...> ttypes_v{};

constexpr nothing nothing_v{};
constexpr dummy dummy_v{};
