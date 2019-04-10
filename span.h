#pragma once

#include <iterator>
#include <type_traits>

#include "util.h"

#pragma warning(push)
#pragma warning(disable : 26481) // Don't use pointer arithmetic. Use span instead

template <class T>
class span
{
public:
    using value_type             = T;
    using pointer                = T *;
    using const_pointer          = const T*;
    using reference              = T &;
    using const_reference        = const T &;
    using const_iterator         = const_pointer;
    using iterator               = pointer;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;

    constexpr span() noexcept
        : data_{ nullptr }
        , size_{ 0_z }
    {}

    constexpr span(pointer data, size_type size) noexcept
        : data_{ data }
        , size_{ size }
    {}

    constexpr span(const span&) noexcept = default;
    constexpr span& operator=(const span&) noexcept = default;

    template<class Container, class = decltype(std::data(std::declval<Container>())), class = decltype(std::size(std::declval<Container>()))>
    constexpr span(Container & c) noexcept
        : data_{ std::data(c) }
        , size_{ std::size(c) }
    {}

    constexpr iterator begin() const noexcept
    {
        return data_;
    }

    constexpr iterator end() const noexcept
    {
        return data_ + size_;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator{ end() };
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator{ begin() };
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    constexpr size_type size() const noexcept
    {
        return size_;
    }

    constexpr size_type length() const noexcept
    {
        return size_;
    }

    constexpr bool empty() const noexcept
    {
        return !size_;
    }

    constexpr pointer data() const noexcept
    {
        return data_;
    }

    constexpr reference operator[](const size_type index) const noexcept
    {
        return data_[index];
    }

    constexpr reference front() const noexcept
    {
        return *data_;
    }

    constexpr reference back() const noexcept
    {
        return data_[size_ - 1_z];
    }

    constexpr span left(size_t size) const noexcept
    {
        return { data_, size };
    }

    constexpr span right(size_t size) const noexcept
    {
        return { data_ + size_ - size, size };
    }

    constexpr span mid(size_t pos, size_t size) const
    {
        return { data_ + pos, size };
    }

private:
    pointer data_;
    size_type size_;
};

#pragma warning(pop)
