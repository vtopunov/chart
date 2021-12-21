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


constexpr size_t operator "" _uz(unsigned long long value) noexcept
{ 
    return value;
}


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

template<class T>
inline constexpr T zero_v{};

template<class T>
constexpr bool is_positive(const T& value) noexcept
{
    return zero_v<T> < value;
}

template<class T>
constexpr bool is_negative(const T& value) noexcept
{
    return value < zero_v<T>;
}

template<class T> [[nodiscard]]
constexpr T constexpr_abs(T value) noexcept
{
    if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
    {
        return value;
    }
    else
    {
        return is_negative(value) ? -value : value;
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

template<bool test, class T>
using add_pointer_if_t = std::conditional_t<test, std::add_pointer_t<T>, T>;

template<class S, class D>
using copy_const_t = add_const_if_t<std::is_const_v<S>, D>;

template<class S, class D>
using copy_pointer_t = add_pointer_if_t<std::is_pointer_v<S>, D>;

template<class Value, class Old, class New>
using replace_t = std::conditional_t<std::is_same_v<Value, Old>, New, Value>;

template<bool Test, class T>
struct make_unsigned_opt0
{
    using type = std::make_unsigned_t<T>;
};

template<class T>
struct make_unsigned_opt0<false, T>
{
    using type = T;
};

template<class T>
struct make_unsigned_opt : make_unsigned_opt0<std::is_integral_v<T>, T>
{};

template<class T>
using make_unsigned_opt_t = typename make_unsigned_opt<T>::type;


template<class T> [[nodiscard]]
constexpr T* as_pointer(T* ptr) noexcept
{
    return ptr;
}

template<class T> [[nodiscard]]
constexpr std::add_const_t<std::add_const_t<T>*> as_const_pointer(T* ptr) noexcept
{
    return ptr;
}

template <class T> [[nodiscard]]
constexpr T& as_reference(T& value) noexcept
{
    return value;
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
void as_mutable(const T&&) = delete;

#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 26472) //  Don't use a static_cast for arithmetic conversions. Use brace initialization, narrow_cast or narrow

template<class Target, class Source>
constexpr bool is_unsigned2_v = std::is_unsigned_v<Source> && std::is_unsigned_v<Target>;

template<class word, class dword> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<word, dword>, word> hi_cast(dword dw) noexcept
{
    if constexpr (sizeof(dword) > sizeof(word))
    {
        constexpr auto nbits = 8_uz * (sizeof(dword) - sizeof(word));

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
    static_assert(mul > 0_uz);

    [[maybe_unused]]
    constexpr size_t overflow = numeric_max_v<size_t> / mul;

    D_ASSERT(size <= overflow);

    return size * mul;
}

template<size_t align> [[nodiscard]]
constexpr size_t size_align(size_t size) noexcept
{
    static_assert(align > 0_uz);

    constexpr auto rem = align - 1_uz;
    static_assert((align & rem) == 0_uz);

    constexpr auto mask = ~rem;
    return (size + rem) & mask;
}


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

template<class L, class R>
constexpr auto swap(L& left, R& right) noexcept -> decltype(private_detail_swap::swap(left, right))
{
    return private_detail_swap::swap(left, right);
}

template<class L, class R = L>
class temp_swap
{
public:
    D_DISABLE_COPY_MOVE(temp_swap);

    constexpr temp_swap(L& left, R& right) noexcept
        : left_{ left }
        , right_{ right }
    {
        ::swap(left_, right_);
    }

    constexpr ~temp_swap() noexcept
    {
        ::swap(right_, left_);
    }

    [[nodiscard]]
    constexpr L& left() const noexcept
    {
        return left_;
    }

    [[nodiscard]]
    constexpr R& right() const noexcept
    {
        return right_;
    }

private:
    L& left_;
    R& right_;
};

template<class L>
temp_swap(L&, L&)->temp_swap<L>;

template<class L, class R>
temp_swap(L&, R&)->temp_swap<L, R>;
