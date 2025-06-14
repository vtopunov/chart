#pragma once

#include <core/view.h>


template<class T>
struct remove_optional_reference_wrapper
{
    using type = T;
};

template<class T>
struct remove_optional_reference_wrapper<optional_reference_wrapper<T>>
{
    using type = T;
};

template<class T>
struct remove_optional_reference_wrapper<const optional_reference_wrapper<T>>
{
    using type = T;
};

template <class T>
using remove_optional_reference_wrapper_t = typename remove_optional_reference_wrapper<T>::type;

template<class T>
[[nodiscard]] constexpr std::add_lvalue_reference_t<remove_optional_reference_wrapper_t<T>> unorefwrap(T& value) noexcept
{
    return value;
}


template<class T>
class optional_reference_wrapper
{
public:
    static_assert(!std::is_reference_v<T>);

    using type = T;

    constexpr optional_reference_wrapper() noexcept
        : data_{ nullptr }
    {}

    constexpr optional_reference_wrapper(T& value) noexcept
        : data_{ std::addressof(value) }
    {}

    [[nodiscard]]
    constexpr operator T& () const noexcept
    {
        return get();
    }

    [[nodiscard]]
    constexpr T& get() const noexcept
    {
        D_ASSERT(has_value());
        return *data_;
    }

    template <class... Args>
    [[nodiscard]] constexpr auto operator()(Args&&... args) const noexcept
        -> decltype(std::declval<T&>()(static_cast<Args&&>(args)...))
    {
        return get()(static_cast<Args&&>(args)...);
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    [[nodiscard]]
    constexpr bool has_value() const noexcept
    {
        return nullptr != data_;
    }

private:
    T* data_;
};


template <class T>
[[nodiscard]] constexpr optional_reference_wrapper<T> oref(T& value) noexcept 
{
    return value;
}

template <class T>
[[nodiscard]] constexpr optional_reference_wrapper<T> oref(optional_reference_wrapper<T> value) noexcept 
{
    return value;
}

template <class T>
void oref(const T&&) = delete;

template <class T>
[[nodiscard]] constexpr optional_reference_wrapper<const T> const_oref(const T& value) noexcept 
{
    return value;
}

template <class T>
[[nodiscard]] constexpr optional_reference_wrapper<const T> const_oref(optional_reference_wrapper<T> value) noexcept 
{
    return value;
}

template <class T>
void const_oref(const T&&) = delete;


template<class T>
using ref_wrap_if_need_t = std::conditional_t<is_view_by_copy_v<T>, T, optional_reference_wrapper<T>>;