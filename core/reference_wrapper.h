#pragma once

#include <core/invoke.h>


template<class T>
class reference_wrapper final
{
public:
    static_assert(!std::is_reference_v<T>);

    using null_type = nullref_t;
    using type = T;

    constexpr explicit reference_wrapper() noexcept = default;
    D_DEFAULT_COPYMOVE_CA(reference_wrapper);
    D_DEFAULT_EQ_OP(reference_wrapper);

    constexpr reference_wrapper(nullref_t) noexcept
        : data_{ nullptr }
    {}

    constexpr reference_wrapper(T& value) noexcept
        : data_{ std::addressof(value) }
    {}

    constexpr reference_wrapper& operator = (nullref_t) noexcept
    {
        reset();
        return *this;
    }

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
        -> decltype(::invoke(get(), std::forward<Args>(args)...))
    {
        return ::invoke(get(), std::forward<Args>(args)...);
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

    constexpr void reset() noexcept
    {
        data_ = nullptr;
    }

private:
    T* data_;
};


namespace private_detail_reference_wrapper
{
    template<class Default, class T>
    struct remove_reference_wrapper_helprer
    {
        using type = std::remove_reference_t<Default>;
    };

    template<class Default, class T>
    struct remove_reference_wrapper_helprer<Default, reference_wrapper<T>>
    {
        using type = T;
    };
}

template <class T>
using remove_reference_wrapper_t = typename private_detail_reference_wrapper::remove_reference_wrapper_helprer<T, std::remove_cvref_t<T>>::type;

template<class T>
[[nodiscard]] constexpr std::add_lvalue_reference_t<remove_reference_wrapper_t<T>> unrefwrap(T&& value) noexcept
{
    return value;
}

template <class T>
[[nodiscard]] constexpr reference_wrapper<remove_reference_wrapper_t<T>> ref(T&& value) noexcept
{
    return std::forward<T>(value);
}

template <class T>
[[nodiscard]] constexpr reference_wrapper<const remove_reference_wrapper_t<T>> cref(const T& value) noexcept
{
    return value;
}