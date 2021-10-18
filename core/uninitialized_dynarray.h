#pragma once

#include <cstdlib>
#include <limits>
#include <utility>

template<class T> [[nodiscard]]
T* typed_memory_allocation(size_t size) noexcept
{
    constexpr size_t type_size = sizeof(T);
    constexpr auto max_size = std::numeric_limits<size_t>::max();
    constexpr size_t overflow = max_size / type_size;

    T* result{ nullptr };

    if (size <= overflow)
    {
#pragma warning(push)
#pragma warning(disable : 26408) // Avoid malloc and free

        using std::malloc;

        result = static_cast<T*>(malloc(type_size * size));

#pragma warning(pop)
    }

    return result;
}


template<class T>
class uninitialized_dynarray
{
public:
    using size_type = size_t;
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;

    constexpr uninitialized_dynarray() noexcept = default;

    constexpr uninitialized_dynarray(uninitialized_dynarray&& right) noexcept
        : data_{ std::exchange(right.data_, nullptr) }
        , size_{ std::exchange(right.size_, 0u) }
    {}

    explicit uninitialized_dynarray(size_type size) noexcept
        : data_{ typed_memory_allocation<T>(size) }
        , size_{ size }
    {}

    uninitialized_dynarray(const uninitialized_dynarray&) noexcept = delete;

    uninitialized_dynarray& operator = (const uninitialized_dynarray&) noexcept = delete;

    constexpr uninitialized_dynarray& operator = (uninitialized_dynarray&& right) noexcept
    {
        swap(right);
        return *this;
    }

    ~uninitialized_dynarray() noexcept
    {
#pragma warning(push)
#pragma warning(disable : 26408) // Avoid malloc and free

        using std::free;

        free(data_);

#pragma warning(pop)
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !!data_;
    }

    constexpr void swap(uninitialized_dynarray& right) noexcept
    {
        std::swap(data_, right.data_);
        std::swap(size_, right.size_);
    }

    void reset() noexcept
    {
        [[maybe_unused]]
        const uninitialized_dynarray temp{ std::move(*this) };
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]]
    constexpr pointer data() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return _end();
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept
    {
        return cbegin();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept
    {
        return cend();
    }

    [[nodiscard]]
    constexpr iterator begin() noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr iterator end() noexcept
    {
        return const_cast<pointer>(_end());
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        return *data_;
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        return *(_end() - 1u);
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept
    {
        return cfront();
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept
    {
        return cback();
    }

    [[nodiscard]]
    constexpr reference front() noexcept
    {
        return const_cast<reference>(cfront());
    }

    [[nodiscard]]
    constexpr reference back() noexcept
    {
        return const_cast<reference>(cback());
    }

    [[nodiscard]]
    constexpr const_reference operator[](size_type index) const noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr reference operator[](size_type index) noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr const_reference cvalue(size_type index) const noexcept
    {
        return data_[index];
    }

    [[nodiscard]]
    constexpr const_reference value(size_type index) const noexcept
    {
        return cvalue(index);
    }

    [[nodiscard]]
    constexpr reference value(size_type index) noexcept
    {
        return const_cast<reference>(cvalue(index));
    }

private:
    [[nodiscard]]
    constexpr const_pointer _end() const noexcept
    {
        return data_ + size_;
    }

private:
    pointer data_{ nullptr };
    size_type size_{ 0u };
};