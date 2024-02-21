#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>

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


struct nulltype_construct_t
{};

constexpr nulltype_construct_t nulltype_construct{};

struct nullmem_t 
{
    constexpr explicit nullmem_t(nulltype_construct_t) noexcept
    {}
};

constexpr nullmem_t nullmem{ nulltype_construct };


template<class T>
struct basic_intrusive_node;

template<class T>
struct intrusive_node_object;

template<class T>
class intrusive_list_ref;

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

using tintf_t = float;

template<class T>
struct rgba_color;

using rgba_color32_t = rgba_color<u8tint_t>;
using rgba_colorf_t = rgba_color<tintf_t>;

using doublemax_t = long double;

template<class T>
struct rational;


template<class T>
class buffer;

static_assert(1u == sizeof(std::byte));
using buffer_t = buffer<std::byte>;


template<class T>
class optional_reference_wrapper;


template<class C>
using decl_value_type_t = typename C::value_type;

template<class T>
using decl_view_type_t = typename T::view_type;

template<class T>
using decl_null_type_t = typename T::null_type;

template<class T>
using decl_resource_type_t = typename T::resource_type;


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