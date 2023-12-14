#pragma once

#include <functional>

#include <core/fwd.h>
#include <core/assert.h>
#include <core/type_traits.h>


template<class T>
class optional_reference_wrapper
{
public:
    using type = T;

    constexpr optional_reference_wrapper() noexcept
        : data_{ nullptr }
    {}

    constexpr optional_reference_wrapper(T& value) noexcept
        : data_{ std::addressof(value) }
    {}

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
    constexpr auto operator()(Args&&... args) const noexcept
        -> decltype(std::invoke(std::declval<T&>(), static_cast<Args&&>(args)...))
    {
        return std::invoke(get(), static_cast<Args&&>(args)...);
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


namespace private_detail_remove_reference_wrapper
{
    template<class T, class R>
    struct remove_reference_wrapper_helper
    {
        using type = T;
    };

    template<class T, class R>
    struct remove_reference_wrapper_helper<T, std::reference_wrapper<R>>
    {
        using type = R;
    };

    template<class T, class R>
    struct remove_reference_wrapper_helper<T, optional_reference_wrapper<R>>
    {
        using type = R;
    };

    template<class T>
    struct remove_reference_wrapper : remove_reference_wrapper_helper<T, T>
    {};

    template<class T>
    struct remove_reference_wrapper<const T> : remove_reference_wrapper_helper<const T, T>
    {};

    template <class T>
    using remove_reference_wrapper_t = typename remove_reference_wrapper<T>::type;
}

using private_detail_remove_reference_wrapper::remove_reference_wrapper;
using private_detail_remove_reference_wrapper::remove_reference_wrapper_t;


template<class T>
[[nodiscard]] constexpr std::add_lvalue_reference_t<std::remove_reference_t<remove_reference_wrapper_t<T>>> unrefwrap(T& value) noexcept
{
    return value;
}


template<typename T>
class unique_function
{
public:
    using function_type = std::function<T>;

    unique_function() noexcept = default;

    unique_function(std::nullptr_t) noexcept
        : fn_{ nullptr }
    {}

    template<typename Fn>
    unique_function(Fn&& fn) noexcept
        : fn_{ wrapper<Fn>{ std::forward<Fn>(fn) } }
    {}

    unique_function(unique_function&&) noexcept = default;

    unique_function& operator=(unique_function&&) noexcept = default;

    unique_function(const unique_function&) noexcept = delete;

    unique_function& operator=(const unique_function&) noexcept = delete;

    unique_function& operator=(std::nullptr_t) noexcept
    {
        fn_ = nullptr;
        return *this;
    }

    template<typename Fn>
    unique_function& operator=(Fn&& fn) noexcept
    {
        fn_ = wrapper<Fn>{ std::forward<Fn>(fn) };
        return *this;
    }

    void swap(unique_function& right) noexcept
    {
        fn_.swap(right.fn_);
    }

    explicit operator bool() const noexcept
    {
        return !!fn_;
    }

    template<typename... Args>
    auto operator()(Args&&... args) const noexcept
    {
        return fn_(std::forward<Args>(args)...);
    }

private:
    template<typename Fn>
    class wrapper
    {
    public:
        wrapper(Fn&& fn) noexcept
            : fn_(std::forward<Fn>(fn))
        {}

        wrapper(wrapper&&) noexcept = default;

        wrapper& operator=(wrapper&&) noexcept = default;

        wrapper(const wrapper& rhs) noexcept
            : fn_(const_cast<Fn&&>(rhs.fn_))
        {
            D_ASSERT(!"dummy copy constructor");
        }

        wrapper& operator=(wrapper&) noexcept
        {
            D_ASSERT(!"dummy copy assignment");
            return *this;
        }

        template<typename... Args>
        auto operator()(Args&&... args) noexcept
        {
            return fn_(std::forward<Args>(args)...);
        }

    private:
        Fn fn_;
    };

private:
    function_type fn_;
};

template<class T>
[[nodiscard]] bool operator!=(const unique_function<T>& other, nullptr_t) noexcept
{
    return !!other;
}

template<class T>
[[nodiscard]] bool operator!=(nullptr_t, const unique_function<T>& other) noexcept
{
    return other != nullptr;
}

template<class T>
[[nodiscard]] bool operator==(const unique_function<T>& other, nullptr_t) noexcept
{
    return !other;
}

template<class T>
[[nodiscard]] bool operator==(nullptr_t, const unique_function<T>& other) noexcept
{
    return other == nullptr;
}


struct nothing
{
    template<class... Args>
    constexpr void operator () (Args&&...) const noexcept
    {}
};