#pragma once

#include <functional>

#include <core/reference_wrapper.h>


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
