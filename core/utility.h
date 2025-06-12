#pragma once

#include <utility>
#include <iterator>

#include <core/limits.h>


template<class T>
[[nodiscard]] constexpr T* as_pointer(T* ptr) noexcept
{
    return ptr;
}

template<class T>
[[nodiscard]] constexpr const T* const as_const_pointer(const T* ptr) noexcept
{
    return ptr;
}

template<class T>
[[nodiscard]] constexpr T* as_mutable_pointer(const T* ptr) noexcept
{
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast);
    return const_cast<T*>(ptr);
    D_WARNING_POP;
}

template <class T>
[[nodiscard]] constexpr T& as_reference(T& value) noexcept
{
    return value;
}

template <class T>
[[nodiscard]] constexpr T& as_mutable(const T& value) noexcept
{
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast);
    return const_cast<T&>(value);
    D_WARNING_POP;
}


namespace container_detection
{
    template<class C>
    using decl_std_data_pointer_t= decltype(as_pointer(std::data(std::declval<C&>())));

    template<class C>
    using decl_std_data_value_t = std::remove_pointer_t<decl_std_data_pointer_t<C>>;

    template<class C>
    using decl_std_size_t = decltype(std::size(std::declval<C&>()));

    template<class T>
    using std_data_value_type_type = enable_if_detected<decl_std_data_value_t, T>;

    template<class T>
    using value_type_type = enable_if_detected_or<std_data_value_type_type<T>, decl_value_type_t, T>;

    template<class C>
    using value_type_t = typename value_type_type<C>::type;

    template<class C>
    using has_std_size = is_detected<decl_std_size_t, C>;

    template<class C>
    using has_std_data_pointer = is_detected<decl_std_data_pointer_t, C>;

    template<class C, class ToPointer>
    using is_std_data_convertible = is_detected_convertible<ToPointer, decl_std_data_pointer_t, C>;
}

using namespace container_detection;


namespace private_detail_string_char
{
    template<class C>
    struct string_char_type0
    {
        using method_type = std::conditional_t<
            std::is_pointer_v<C>,
            std::decay<std::remove_pointer_t<C>>,
            value_type_type<C>
        >;

        using type = typename method_type::type;
    };

    template<class C>
    struct string_char_type
    {
        using type = typename string_char_type0<std::decay_t<C>>::type;
    };

    template<class C>
    using string_char_t = typename string_char_type<C>::type;
}

using private_detail_string_char::string_char_type;
using private_detail_string_char::string_char_t;


namespace private_detail_u_swap
{
    using namespace ordered_overload;

    template<class L, class R>
    constexpr auto u_swap_impl(L& left, R& right, std::false_type, _order<_3>) noexcept -> decltype((void)(std::swap<R>(right, left)))
    {
        std::swap<R>(right, left);
    }

    template<class L, class R>
    constexpr auto u_swap_impl(L& left, R& right, std::false_type, _order<_2>) noexcept -> decltype((void)(std::swap<L>(left, right)))
    {
        std::swap<L>(left, right);
    }

    template<class L, class R>
    constexpr auto u_swap_impl(L& left, R& right, std::false_type, _order<_1>) noexcept -> decltype((void)(right.swap(left)))
    {
        right.swap(left);
    }

    template<class L, class R>
    constexpr auto u_swap_impl(L& left, R& right, std::false_type, _order<_0>) noexcept -> decltype((void)(left.swap(right)))
    {
        left.swap(right);
    }

    template<class L, class R>
    constexpr auto u_swap_impl(L& left, R& right, std::true_type, _order<_3>) noexcept -> decltype((void)(std::swap<L>(left, right)))
    {
        std::swap<L>(left, right);
    }

    template<class L, class R>
    constexpr auto u_swap_impl(L& left, R& right, std::true_type, _order<_2>) noexcept -> decltype((void)(std::swap<R>(right, left)))
    {
        std::swap<R>(right, left);
    }

    template<class L, class R>
    constexpr auto u_swap_impl(L& left, R& right, std::true_type, _order<_1>) noexcept -> decltype((void)(left.swap(right)))
    {
        left.swap(right);
    }

    template<class L, class R>
    constexpr auto u_swap_impl(L& left, R& right, std::true_type, _order<_0>) noexcept -> decltype((void)(right.swap(left)))
    {
        right.swap(left);
    }

    template<class L, class R>
    constexpr std::disjunction
        <
        std::conjunction<std::is_base_of<L, R>, std::negation<std::is_base_of<R, L>>>,
        std::conjunction<std::is_convertible<R, L>, std::negation<std::is_convertible<L, R>>>
        > R_is_full_declval;

    template<class L, class R>
    constexpr auto u_swap(L& left, R& right) noexcept -> decltype((void)u_swap_impl(left, right, R_is_full_declval<L, R>, _start))
    {
        u_swap_impl(left, right, R_is_full_declval<L, R>, _start);
    }
}

using private_detail_u_swap::u_swap;

template<class T>
[[nodiscard]] constexpr bool is_null_or_zterm(const T* string) noexcept
{
    return !string || !*string;
}

template<size_t mul>
[[nodiscard]] constexpr bool has_size_mul(const size_t size) noexcept
{
    static_assert(mul > 0_uz);
    constexpr auto overflow = size_maxi / mul;
    return size <= overflow;
}

template<size_t add>
[[nodiscard]] constexpr bool has_size_add(const size_t size) noexcept
{
    constexpr auto overflow = size_maxi - add;
    return size <= overflow;
}

template<size_t mul>
[[nodiscard]] constexpr size_t size_mul(const size_t size) noexcept
{
    static_assert(mul > 0_uz);
    D_ASSERT(has_size_mul<mul>(size));
    return size * mul;
}

template<size_t add>
[[nodiscard]] constexpr size_t size_add(const size_t size) noexcept
{
    D_ASSERT(has_size_add<add>(size));
    return size + add;
}

template<size_t mul>
[[nodiscard]] constexpr size_t size_mul_or_max(const size_t size) noexcept
{
    if(has_size_mul<mul>(size)) [[likely]]
    {
        return size * mul;
    }

    return size_maxi;
}

template<size_t add>
[[nodiscard]] constexpr size_t size_add_or_max(const size_t size) noexcept
{
    if(has_size_add<add>(size)) [[likely]]
    {
        return size + add;
    }

    return size_maxi;
}

template<size_t align>
[[nodiscard]] constexpr size_t size_align(const size_t size) noexcept
{
    static_assert(align > 0_uz);

    constexpr auto rem = align - 1_uz;
    static_assert((align & rem) == 0_uz);

    constexpr auto mask = ~rem;
    return (size + rem) & mask;
}

template<class L, class R>
[[nodiscard]] constexpr enable_if_detected_and_t<std::common_type_t<L, R>, decl_less_op_t, R, L> u_min
(
    const L& a,
    const R& b
) noexcept
{
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_signed_unsigned_mismatch);
    return (b < a) ? b : a;
    D_WARNING_POP;
}

template<class L, class R>
[[nodiscard]] constexpr enable_if_detected_and_t<std::common_type_t<L, R>, decl_less_op_t, L, R> u_max
(
    const L& a,
    const R& b
) noexcept
{
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_signed_unsigned_mismatch);
    return (a < b) ? b : a;
    D_WARNING_POP;
}


