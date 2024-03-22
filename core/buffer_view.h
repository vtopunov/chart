#pragma once

#include <string_view>

#include <core/span.h>


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
constexpr bool is_buffer_view_v = is_buffer_view<T>::value;

namespace private_detail_size_bytes
{
    using namespace ordered_overload;

    template<class T>
    [[nodiscard]] constexpr size_t size_of() noexcept
    {
        using type_t = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<type_t, void>)
        {
            return 1_uz;
        }
        else
        {
            return sizeof(type_t);
        }
    }

    template<class C>
    [[nodiscard]] constexpr auto value_type_size() -> decltype(size_of<value_type_t<C>>())
    {
        return size_of<value_type_t<C>>();
    }

    template<class C>
    [[nodiscard]] constexpr auto size_bytes_impl(const C& c, _order<_1>) noexcept
        -> decltype(value_type_size<C>(), std::size(c), 0_uz)
    {
        constexpr auto type_size = value_type_size<C>();
        return size_mul<type_size>(narrow<size_t>(std::size(c)));
    }

    template<class C>
    [[nodiscard]] constexpr auto size_bytes_impl(const C& c, _order<_0>) noexcept -> decltype(c.size_bytes())
    {
        return c.size_bytes();
    }

    template<class C>
    [[nodiscard]] constexpr auto size_bytes(const C& c) noexcept -> decltype(size_bytes_impl(c, _start))
    {
        return size_bytes_impl(c, _start);
    }
}

using private_detail_size_bytes::size_bytes;

template<class T>
using size_bytes_t = decltype(size_bytes(std::declval<T&>()));

template<class T>
using is_size_bytes = is_detected<size_bytes_t, T>;

template<class C>
constexpr bool is_size_bytes_v = is_size_bytes<C>::value;

template <class C, class Data>
constexpr bool is_compatible_buffer_v = std::conjunction_v
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
    using const_opt = conditional_add_const_t<immutable, T>;

    template<class T>
    using const_opt_pointer = std::add_pointer_t<const_opt<T>>;;

    template<class T>
    using const_opt_span = span<const_opt<T>>;

    using byte_type = std::byte;
    using value_type = byte_type;
    using element_type = const_opt<value_type>;
    using size_type = size_t;
    using pointer = element_type*;
    using const_pointer = const element_type*;
    using reference = element_type&;
    using const_reference = const element_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using view_type = const_buffer_view;
    using null_type = nullmem_t;
    using data_pointer = const_opt_pointer<void>;
    static_assert(1u == sizeof(byte_type));
    static_assert(1u == sizeof(value_type));

    template<class C>
    static constexpr bool is_compatible_v = is_compatible_buffer_v<C, data_pointer>;

    D_DEFAULT_ALL_CAEQ(basic_buffer_view);

    constexpr basic_buffer_view(null_type) noexcept
        : basic_buffer_view{}
    {}

    constexpr basic_buffer_view(data_pointer data, size_type size) noexcept
        : data_{ data }
        , size_{ size }
    {}

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

    template<class T, size_t n, std::enable_if_t<std::is_convertible_v<T*, data_pointer>, int> = 0>
    constexpr basic_buffer_view(span<T, n> span) noexcept
        : data_{ std::data(span) }
        , size_{ size_bytes(span) }
    {}

    constexpr basic_buffer_view& operator = (null_type nullvalue) noexcept
    {
        return basic_buffer_view::operator=(static_cast<basic_buffer_view>(nullvalue));
    }

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

    template<class T, size_t n>
    constexpr std::enable_if_t<std::is_convertible_v<T*, data_pointer>, basic_buffer_view&> operator = (span<T, n> span) noexcept
    {
        data_ = std::data(span);
        size_ = size_bytes(span);
        return *this;
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !!size_;
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
    [[nodiscard]] constexpr const_opt_span<T> as_span() const noexcept
    {
        return { _as_ptr<T>(), _count_for<T>() };
    }

    template<class T>
    [[nodiscard]] constexpr std::basic_string_view<T> as_str() const noexcept
    {
        return { _as_ptr<std::add_const_t<T>>(), _count_for<T>() };
    }

    [[nodiscard]]
    constexpr const_opt_span<byte_type> as_bytes() const noexcept
    {
        return { _as_bytes_ptr(), size() };
    }

    [[nodiscard]]
    constexpr reference value(size_type index) const noexcept
    {
        return _as_bytes_ptr()[index];
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
        return const_cast<reference>(cback());
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        return *(cend() - 1_uz);
    }

    [[nodiscard]]
    constexpr iterator begin() const noexcept
    {
        return _as_bytes_ptr();
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    [[nodiscard]]
    constexpr iterator end() const noexcept
    {
        return const_cast<iterator>(cend());
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return _as_bytes_ptr() + size_;
    }

private:
    template<class T>
    [[nodiscard]] constexpr size_t _count_for() const noexcept
    {
        static_assert(!std::is_reference_v<T>);
        return size() / sizeof(T);
    }

    template<class T>
    [[nodiscard]] constexpr const_opt_pointer<T> _as_ptr() const noexcept
    {
        return static_cast<const_opt_pointer<T>>(data());
    }

    [[nodiscard]]
    constexpr const_opt_pointer<byte_type> _as_bytes_ptr() const noexcept
    {
        return _as_ptr<byte_type>();
    }

private:
    data_pointer data_{ nullptr };
    size_type size_{ 0_uz };
};

template<class T>
[[nodiscard]] constexpr span<const T> to_span(const const_buffer_view buffer) noexcept
{
    return buffer.template as_span<T>();
}

template<class T>
[[nodiscard]] constexpr span<T> to_span(const buffer_view buffer) noexcept
{
    return buffer.template as_span<T>();
}

template<class T>
[[nodiscard]] constexpr std::basic_string_view<T> to_string_view(const const_buffer_view buffer) noexcept
{
    return buffer.template as_str<T>();
}

inline void zero_memory(buffer_view buffer) noexcept
{
    memset(buffer.data(), 0, buffer.size());
}