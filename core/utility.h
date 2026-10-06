#pragma once

#include <utility>
#include <iterator>

#include <core/narrow.h>


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
[[nodiscard]] constexpr T& as_mutable(const T& value) noexcept
{
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast);
    return const_cast<T&>(value);
    D_WARNING_POP;
}

template <class T>
[[nodiscard]] constexpr std::add_lvalue_reference_t<T> as_lref(T&& value) noexcept
{
    return value;
}


template <class T, class... Args>
constexpr void construct_at(T* const location, Args&&... args) noexcept
{
    D_ASSERT(nullptr != location);

    if constexpr (is_brace_constructible_v<T, Args...>)
    {
        new (location) T{ std::forward<Args>(args)... };
    }
    else
    {
        new (location) T(std::forward<Args>(args)...);
    }
}

template <class T, class... Args>
constexpr void move_construct_at(T* const location, T&& value) noexcept
{
    D_ASSERT(nullptr != location);
    D_ASSERT(location != std::addressof(value));
    new (location) T(std::move(value));
}

template <class T>
constexpr void destroy_at(const T* const location) noexcept 
{
    location->~T();
}

template <class T>
constexpr void destroy(const T& value) noexcept
{
    value.~T();
}

namespace private_detail_utility
{
    namespace private_detail_cdata
    {
        using namespace ordered_overload;

        template<class T>
        [[nodiscard]] constexpr auto cdata_impl(const T& value, _order<_2>) noexcept -> decltype(std::data(value))
        {
            return std::data(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto cdata_impl(const T& value, _order<_1>) noexcept -> decltype(as_const_pointer(std::data(value)))
        {
            return std::data(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto cdata_impl(const T& value, _order<_0>) noexcept -> decltype(value.cdata())
        {
            return value.cdata();
        }

        template<class T>
        [[nodiscard]] constexpr auto cdata(const T& value) noexcept -> decltype(cdata_impl(value, _start))
        {
            return cdata_impl(value, _start);
        }
    }
}

using private_detail_utility::private_detail_cdata::cdata;


namespace container_detection
{
    template<class C>
    using decl_std_data_pointer_t = decltype(as_pointer(std::data(std::declval<C&>())));

    template<class C>
    using decl_std_data_value_t = std::remove_pointer_t<decl_std_data_pointer_t<C>>;

    template<class C>
    using decl_std_size_t = decltype(std::size(std::declval<C&>()));

    template<class C>
    using decl_const_opt_value_type_t = copy_const_t<C, decl_value_type_t<C>>;

    template<class C>
    using container_value_type_type = enable_if_detected<decl_const_opt_value_type_t, C>;

    template<class C>
    using value_type_type = enable_if_detected_or<container_value_type_type<C>, decl_std_data_value_t, C>;

    template<class C>
    using value_type_t = typename value_type_type<C>::type;

    template<class C>
    using has_std_size = is_detected<decl_std_size_t, C>;

    template<class C>
    using has_std_data_pointer = is_detected<decl_std_data_pointer_t, C>;

    template<class C>
    using has_container_value_type = is_detected<decl_value_type_t, C>;

    template<class C>
    using has_value_type = std::disjunction<has_std_data_pointer<C>, has_container_value_type<C>>;

    template<class Target, class Container>
    using has_std_data_compatible = is_const_convertible<
        detected_or_t<ttypes<Target>, decl_std_data_value_t, Container>,
        Target
    >;

    template<class C>
    using has_mutable_std_data = std::negation<std::is_const<detected_or_t<std::add_const_t<dummy>, decl_std_data_value_t, C>>>;

    template<bool immutable, class Container>
    using has_std_data_void_compatible = std::disjunction<std::bool_constant<immutable>, has_mutable_std_data<Container>>;
}

using namespace container_detection;


namespace private_detail_utility
{
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

            using type = std::remove_const_t<typename method_type::type>;
        };

        template<class C>
        struct string_char_type
        {
            using type = typename string_char_type0<std::decay_t<C>>::type;
        };

        template<class C>
        using string_char_t = typename string_char_type<C>::type;
    }
}

using private_detail_utility::private_detail_string_char::string_char_type;
using private_detail_utility::private_detail_string_char::string_char_t;


namespace private_detail_utility
{
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
}

using private_detail_utility::private_detail_u_swap::u_swap;


template<class T>
[[nodiscard]] constexpr bool is_null_or_zfront(const T* string) noexcept
{
    return !string || !*string;
}


constexpr auto size_overflow_maxi = narrow<size_t>(numeric_max_v<ptrdiff_t>);
static_assert(size_overflow_maxi < numeric_max_v<size_t>);

template<size_t mul>
[[nodiscard]] constexpr bool has_size_mul(const size_t size) noexcept
{
    static_assert(mul > 0_uz);
    constexpr auto overflow = size_overflow_maxi / mul;
    return size <= overflow;
}

template<size_t add>
[[nodiscard]] constexpr bool has_size_add(const size_t size) noexcept
{
    static_assert(add <= size_overflow_maxi);
    constexpr auto overflow = size_overflow_maxi - add;
    return size <= overflow;
}

template<size_t mul, size_t add>
[[nodiscard]] constexpr bool has_size_mul_add(const size_t size) noexcept
{
    static_assert(mul > 0_uz);
    static_assert(add <= size_overflow_maxi);
    constexpr auto overflow = (size_overflow_maxi - add) / mul;
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
    if (has_size_mul<mul>(size)) [[likely]]
    {
        return size * mul;
    }

    return size_overflow_maxi;
}

template<size_t add>
[[nodiscard]] constexpr size_t size_add_or_max(const size_t size) noexcept
{
    if (has_size_add<add>(size)) [[likely]]
    {
        return size + add;
    }

    return size_overflow_maxi;
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_unsigned_v<T>, T> ceil_div(T value, T div) noexcept
{
    return (value / div) + !!(value % div);
}

template<size_t align>
[[nodiscard]] constexpr size_t size_align(const size_t size) noexcept
{
    static_assert(align > 0_uz);

    constexpr auto rem = align - 1_uz;
    static_assert((align & rem) == 0_uz);

    constexpr auto mask = ~rem;
    return size_add<rem>(size) & mask;
}

template<class M, class T>
[[nodiscard]] constexpr bool test_no_unique_address(M T::* m) noexcept
{
    constexpr union { char ini; M m; T o; } test{ '\0' };
    return std::addressof(test.m) == std::addressof(test.o.*m);
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

template<size_t ElementSize, size_t NewElementSize>
[[nodiscard]] constexpr size_t reinterpret_size(const size_t size) noexcept
{
    static_assert(0_uz < ElementSize);
    static_assert(0_uz < NewElementSize);

    constexpr auto element_size = ElementSize;
    constexpr auto new_element_size = NewElementSize;

    if constexpr (new_element_size > element_size)
    {
        if constexpr (!(new_element_size % element_size))
        {
            constexpr auto new_interpretable_element_size = new_element_size / element_size;
            return size / new_interpretable_element_size;
        }
        else
        {
            return size_mul<element_size>(size) / new_element_size;
        }
    }
    else
    {
        if constexpr (element_size == new_element_size)
        {
            return size;
        }
        else
        {
            if constexpr (!(element_size % new_element_size))
            {
                constexpr auto new_interpretable_element_size = element_size / new_element_size;
                return size_mul<new_interpretable_element_size>(size);
            }
            else
            {
                return size_mul<element_size>(size) / new_element_size;
            }
        }
    }
}


namespace private_detail_utility
{
    namespace private_detail_size_bytes
    {
        using namespace ordered_overload;

        template<class C>
        [[nodiscard]] constexpr std::enable_if_t<
            std::conjunction_v<has_std_size<C>, has_value_type<C>>,
            size_t
        > size_bytes_impl(const C& c, _order<_1>) noexcept
        {
            return size_mul<sizeof_v<value_type_t<C>>>(narrow<size_t>(std::size(c)));
        }

        template<class C>
        [[nodiscard]] constexpr auto size_bytes_impl(const C& c, _order<_0>) noexcept -> decltype(c.size_bytes())
        {
            return c.size_bytes();
        }

        template<class C>
        [[nodiscard]] constexpr auto size_bytes(const C& c) noexcept -> decltype(size_bytes_impl(c, _start))
        {
            return size_bytes_impl(c, _start);
        }
    }
}

using private_detail_utility::private_detail_size_bytes::size_bytes;

template<class T>
using decl_size_bytes_t = decltype(size_bytes(std::declval<T&>()));

template<class T>
using has_size_bytes = is_detected<decl_size_bytes_t, T>;

template<class C>
constexpr bool has_size_bytes_v = has_size_bytes<C>::value;


template<class>
struct dummy_end
{};

template<class T>
[[nodiscard]] constexpr auto operator == (const T& value, dummy_end<T>) noexcept -> decltype(!value)
{
    return !value;
}

template<class T>
[[nodiscard]] constexpr auto operator != (const T& value, dummy_end<T>) noexcept -> decltype(!!value)
{
    return !!value;
}

template<class T>
[[nodiscard]] constexpr auto operator == (dummy_end<T>, const T& value) noexcept -> decltype(!value)
{
    return !value;
}

template<class T>
[[nodiscard]] constexpr auto operator != (dummy_end<T>, const T& value) noexcept -> decltype(!!value)
{
    return !!value;
}


struct errno_holder
{
    const int value;

    errno_holder() noexcept
        : value{ errno }
    {}

    D_DISABLE_COPYMOVE_CA(errno_holder);

    ~errno_holder() noexcept
    {
        errno = value;
    }
};