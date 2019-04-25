#pragma once

#include <iterator>
#include <type_traits>

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

    constexpr span() noexcept = default;

    constexpr span(pointer data, size_type size) noexcept
        : span_data{ data }
        , span_size{ size }
    {}

    template<class Container, class = decltype( std::data(std::declval<Container>()) ), class = decltype( std::size(std::declval<Container>()) )>
    constexpr span(Container & c) noexcept
        : span_data{ std::data(c) }
        , span_size{ std::size(c) }
    {}

    constexpr iterator begin() const noexcept
    {
        return span_data;
    }

    constexpr iterator end() const noexcept
    {
        return span_data + span_size;
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
        return span_size;
    }

    constexpr size_type length() const noexcept
    {
        return span_size;
    }

    constexpr bool empty() const noexcept
    {
        return !span_size;
    }

    constexpr pointer data() const noexcept
    {
        return span_data;
    }

    constexpr reference operator[](const size_type index) const noexcept
    {
        return span_data[index];
    }

    constexpr reference front() const noexcept
    {
        return *span_data;
    }

    constexpr reference back() const noexcept
    {
        return span_data[span_size - 1];
    }

    constexpr span left(size_t size) const noexcept
    {
        return { span_data, size };
    }

    constexpr span right(size_t size) const noexcept
    {
        return { span_data + span_size - size, size };
    }

    constexpr span mid(size_t pos, size_t size) const
    {
        return { span_data + pos, size };
    }

private:
    pointer span_data{};
    size_type span_size{};
};

#pragma warning(pop)
