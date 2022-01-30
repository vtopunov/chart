#pragma once

#include <iterator>
#include <span>
#include <string_view>

#include <core/member_detector.h>
#include <core/narrow.h>

template<bool immutable>
class basic_buffer_view;

using buffer_view = basic_buffer_view<false>;
using const_buffer_view = basic_buffer_view<true>;

template <class T>
struct is_buffer_view : std::false_type
{};

template <>
struct is_buffer_view<buffer_view> : std::true_type
{};

template <>
struct is_buffer_view<const_buffer_view> : std::true_type
{};

template <class T>
struct is_buffer_view<const T> : is_buffer_view<T>
{};

template <class T>
inline constexpr bool is_buffer_view_v = is_buffer_view<T>::value;

template<class C>
using data_pointer_t = decltype(as_pointer(std::data(std::declval<C&>())));

template<class C> [[nodiscard]]
constexpr auto size_bytes(const C& c) noexcept 
    -> decltype(as_pointer(std::data(std::declval<C&>())), std::size(std::declval<C&>()), 0_uz)
{
    using data_t = data_pointer_t<C>;
    using value_t = std::remove_cvref_t<std::remove_pointer_t<data_t>>;
    using value_with_size_t = replace_t<value_t, void, std::byte>;
    constexpr size_t type_size = sizeof(value_with_size_t);

    return size_mul<type_size>(narrow_cast<size_t>(std::size(c)));
}

template<class T>
using size_bytes_t = decltype(size_bytes(std::declval<T&>()));

template<class T>
using is_size_bytes = is_detected<size_bytes_t, T>;

template<class C>
inline constexpr bool is_size_bytes_v = is_size_bytes<C>::value;

template<class C, class DataPointer>
struct is_convertible_data : std::is_convertible<data_pointer_t<C>, DataPointer>
{};

template <class C, class Data>
inline constexpr bool is_compatible_buffer_v = std::conjunction_v
<
    std::negation<is_buffer_view<C>>,
    is_size_bytes<C>,
    is_convertible_data<C, Data>
>;

template<bool immutable>
class basic_buffer_view
{
public:
    template<class T>
    using switchable_const = add_const_if_t<immutable, T>;

    using value_type = std::byte;
    using element_type = switchable_const<value_type>;
    using size_type = size_t;
    using pointer = element_type*;
    using const_pointer = const element_type*;
    using reference = element_type&;
    using const_reference = const element_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using data_pointer = switchable_const<void>*;

    template<class C>
    static constexpr bool is_compatible_v = is_compatible_buffer_v<C, data_pointer>;

    constexpr basic_buffer_view() noexcept = default;

    constexpr basic_buffer_view(data_pointer data, size_type size) noexcept
        : data_{ data }
        , size_{ size }
    {}

    constexpr basic_buffer_view(const basic_buffer_view&) noexcept = default;

    template<bool dummy = true, std::enable_if_t<(dummy) && immutable, int> = 0>
    constexpr basic_buffer_view(const buffer_view& buffer) noexcept
        : data_{ buffer.data() }
        , size_{ buffer.size() }
    {}

    template<class C, std::enable_if_t<is_compatible_v<C>, int> = 0>
    constexpr basic_buffer_view(C& container) noexcept
        : data_{ std::data(container) }
        , size_{ size_bytes(container) }
    {}

    template<class T, size_t n, std::enable_if_t<is_compatible_v<std::span<T, n>>, int> = 0>
    constexpr basic_buffer_view(std::span<T, n> span) noexcept
        : data_{ std::data(span) }
        , size_{ size_bytes(span) }
    {}

    constexpr basic_buffer_view& operator = (const basic_buffer_view&) noexcept = default;

    template<bool dummy = true, std::enable_if_t<(dummy) && immutable, int> = 0>
    constexpr basic_buffer_view& operator = (const buffer_view& buffer) noexcept
    {
        data_ = buffer.data();
        size_ = buffer.size();
        return *this;
    }

    template<class C>
    constexpr std::enable_if_t<is_compatible_v<C>, basic_buffer_view&> operator = (C& container) noexcept
    {
        data_ = std::data(container);
        size_ = size_bytes(container);
        return *this;
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !!data();
    }

    [[nodiscard]]
    constexpr data_pointer data() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]]
    constexpr const_buffer_view as_const() const noexcept
    {
        return *this;
    }

    template<class T>
    [[nodiscard]] constexpr switchable_const<T>* as_ptr() const noexcept
    {
        return static_cast<switchable_const<T>*>(data());
    }

    template<class T>
    using span_switchable_const = std::span<switchable_const<T>>;

    template<class T>
    [[nodiscard]] constexpr span_switchable_const<T> as_span() const noexcept
    {
        return { as_ptr<T>(), _count<T>() };
    }

    template<class T>
    [[nodiscard]] constexpr std::basic_string_view<std::remove_const_t<T>> as_str() const noexcept
    {
        return { as_ptr<T>(), _count<T>() };
    }

    [[nodiscard]]
    constexpr switchable_const<std::byte>* as_bytes_ptr() const noexcept
    {
        return as_ptr<std::byte>();
    }

    [[nodiscard]]
    constexpr span_switchable_const<std::byte> as_bytes() const noexcept
    {
        return { as_bytes_ptr(), size() };
    }

    [[nodiscard]]
    constexpr reference value(size_type index) const noexcept
    {
        return as_bytes_ptr()[index];
    }

    [[nodiscard]]
    constexpr reference operator[](size_type index) const noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr reference front() const noexcept
    {
        return value(0_uz);
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        return front();
    }

    [[nodiscard]]
    constexpr reference back() const noexcept
    {
        return *(_end() - 1_uz);
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        return back();
    }

    [[nodiscard]]
    constexpr iterator begin() const noexcept
    {
        return as_bytes_ptr();
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    [[nodiscard]]
    constexpr iterator end() const noexcept
    {
        return _end();
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

private:
    [[nodiscard]]
    constexpr pointer _end() const noexcept
    {
        return as_bytes_ptr() + size_;
    }

    template<class T>
    [[nodiscard]] constexpr size_t _count() const noexcept
    {
        return size() / sizeof(T);
    }

private:
    data_pointer data_{ nullptr };
    size_type size_{ 0_uz };
};