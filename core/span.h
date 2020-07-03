#pragma once

#include <iterator>

#include <core/narrow_cast.h>

template <class, class = void>
struct is_container : std::false_type
{};

template <class T>
struct is_container<T, std::void_t<decltype(std::data(std::declval<T&>())), decltype(std::size(std::declval<T&>()))>>
    : std::true_type
{};

template<class T>
inline constexpr bool is_container_v = is_container<T>::value;

template <class T>
class span;

template <class T>
struct is_span : public std::false_type
{};

template <class T>
struct is_span<span<T>> : public std::true_type
{};

template<class T>
inline constexpr bool is_span_v = is_span<T>::value;

#pragma warning(push)
#pragma warning(disable : 26481) // Don't use pointer arithmetic. Use span instead

template <class T>
class span
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using const_value_type = std::add_const_t<value_type>;

    constexpr span() noexcept = default;

    constexpr span(pointer data, size_type size) noexcept
        : data_{ data }
        , size_{ size }
    {}

    template<class C, class = std::enable_if_t<is_container_v<C> && !is_span_v<C>>>
    constexpr span(C& c) noexcept
        : data_{ std::data(c) }
        , size_{ narrow_cast<size_type>(std::size(c)) }
    {}

    template<
        bool enable_bool = true,
        class = std::enable_if_t<(!std::is_same_v<value_type, const_value_type>&& enable_bool)>
    >
        constexpr operator span<const_value_type>() const noexcept
    {
        return { data_, size_ };
    }

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

    constexpr reverse_iterator rbegin() const noexcept
    {
        return reverse_iterator{ end() };
    }

    constexpr reverse_iterator rend() const noexcept
    {
        return reverse_iterator{ begin() };
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator{ end() };
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator{ begin() };
    }

    constexpr size_type size() const noexcept
    {
        return size_;
    }

    constexpr pointer data() const noexcept
    {
        return data_;
    }

    constexpr reference value(size_type index) const noexcept
    {
        return data_[index];
    }

    constexpr reference operator[](size_type index) const noexcept
    {
        return value(index);
    }

    constexpr reference front() const noexcept
    {
        return value(0u);
    }

    constexpr reference back() const noexcept
    {
        return value(size_ - 1u);
    }

    constexpr span first(size_type size) const noexcept
    {
        return { data_, size };
    }

    constexpr span subspan(size_type pos, size_type size) const noexcept
    {
        return { data_ + pos, size };
    }

    constexpr span last(size_type size) const noexcept
    {
        return { data_ + size_ - size, size };
    }

private:
    pointer data_{ nullptr };
    size_type size_{ 0u };
};

#pragma warning(pop)
