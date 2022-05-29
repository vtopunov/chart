#pragma once

#include <iterator>
#include <span>

#include <core/value_type.h>
#include <core/narrow.h>


template <class, class = void>
struct is_container : std::false_type
{};

template <class C>
struct is_container<C, std::void_t<decl_data_pointer_t<C>, decltype(std::size(std::declval<C&>()))>>
    : std::true_type
{};

template<class C>
inline constexpr bool is_container_v = is_container<C>::value;

template <class T>
class span;

template <class T>
struct is_span : std::false_type
{};

template <class T>
struct is_span<span<T>> : std::true_type
{};

template <class T>
struct is_span<const T> : is_span<T>
{};

template<class C, class DataPointer>
struct is_convertible_data : std::is_convertible<decl_data_pointer_t<C>, DataPointer>
{};

template <class C, class Data>
constexpr bool is_compatible_span_v = std::conjunction_v
<
    std::negation<is_span<C>>,
    is_container_v<C>,
    is_convertible_data<C, Data>
>;


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

    template<class C>
    static constexpr bool is_compatible_v = is_compatible_span_v<C, pointer>;

    constexpr span() noexcept = default;

    constexpr span(pointer data, size_type size) noexcept
        : data_{ data }
        , size_{ size }
    {}

    constexpr span(const span&) noexcept = default;

    template<class U, std::enable_if_t<std::is_convertible_v<U*, pointer>, int> = 0>
    constexpr span(const span<U>& span) noexcept
        : data_{ span.data() }
        , size_{ span.size() }
    {}

    template<class C, std::enable_if_t<is_compatible_v<C>, int> = 0>
    constexpr span(C& c) noexcept
        : data_{ std::data(c) }
        , size_{ narrow_cast<size_type>(std::size(c)) }
    {}


    constexpr span& operator = (const span&) noexcept = default;

    template<class C>
    constexpr std::enable_if_t<is_compatible_v<C>, span&> operator = (C& container) noexcept
    {
        data_ = std::data(container);
        size_ = std::size(container);
        return *this;
    }

    template<class U, size_t n>
    constexpr std::enable_if_t<std::is_convertible_v<U*, pointer>, span&> operator = (span<U> span) noexcept
    {
        data_ = span.data();
        size_ = span.size();
        return *this;
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
