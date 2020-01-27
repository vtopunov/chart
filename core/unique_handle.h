#pragma once

#include <utility>

#include <core/assert.h>

#include "private/handle_private.h"

template <class T>
class unique_handle
{
public:
    using handle_type = T;;
    using view_type = private_handle::view_type_t<handle_type>;

    constexpr unique_handle() noexcept
        : handle_{}
    {}

    constexpr unique_handle(T handle) noexcept
        : handle_{ std::move(handle) }
    {}

    constexpr unique_handle(unique_handle&& right) noexcept
        : handle_{ right.release() }
    {}

    constexpr unique_handle& operator=(unique_handle&& right) noexcept
    {
        swap(right);
        return *this;
    }

    constexpr void swap(unique_handle& right) noexcept
    {
        std::swap(handle_, right.handle_);
    }

    constexpr const handle_type* operator ->() const noexcept
    {
        return &handle_;
    }

    explicit constexpr operator bool() const noexcept
    {
        return is_valid();
    }

    constexpr operator view_type () const noexcept
    {
        return private_handle::view(handle_);
    }

    constexpr bool is_valid() const noexcept
    {
        return private_handle::is_valid(handle_);
    }

    constexpr handle_type release() noexcept
    {
        return std::exchange(handle_, {});
    }

    ~unique_handle() noexcept
    {
        handle_.close();
    }

    unique_handle(const unique_handle&) = delete;

    unique_handle& operator=(const unique_handle&) = delete;

private:
    handle_type handle_;
};

template <class T, class... Types>
constexpr unique_handle<T> make_unique_handle(Types&&... args) noexcept
{
    return T{ std::forward<Types>(args)... };
}


