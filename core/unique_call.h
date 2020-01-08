#pragma once

#include <utility>

template <class T>
class unique_call 
{
public:
    constexpr unique_call() noexcept
        : call_{} 
    {}

    constexpr unique_call(T call) noexcept 
        : call_{ std::move(call) }
    {}

    constexpr unique_call(unique_call&& right) noexcept
        : call_{ std::exchange(right.call_, T{}) }
    {}

    constexpr unique_call& operator=(unique_call&& right) noexcept
    {
        swap(right);
        return *this;
    }

    constexpr void swap(unique_call& right) noexcept 
    {
        std::swap(call_, right.call_);
        return *this;
    }

    constexpr const T* operator ->() const noexcept
    {
        return &call_;
    }

    constexpr const T& get() const noexcept
    {
        return call_;
    }

    ~unique_call() noexcept
    {
        call_();
    }

    unique_call(const unique_call&) = delete;

    unique_call& operator=(const unique_call&) = delete;

private:
    T call_;
};

