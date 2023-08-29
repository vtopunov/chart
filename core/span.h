#pragma once

#include <array>

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

template <class T, size_t>
class span;

template <class T>
struct is_span : std::false_type
{};

template <class T, size_t Extent>
struct is_span<span<T, Extent>> : std::true_type
{};

template <class T>
struct is_span<const T> : is_span<T>
{};

template <class T>
constexpr bool is_span_v = is_span<T>::value;


template<class C, class DataPointer>
struct is_convertible_data : std::is_convertible<decl_data_pointer_t<C>, DataPointer>
{};


constexpr auto dynamic_extent = numeric_max_v<size_t>;

namespace private_detail_extent_constant
{
    template<class C>
    struct extent_constant_impl : std::integral_constant<size_t, dynamic_extent>
    {};

    template<class T, size_t Extent>
    struct extent_constant_impl<std::array<T, Extent>> : std::integral_constant<size_t, Extent>
    {};

    template<class T, size_t Extent>
    struct extent_constant_impl<span<T, Extent>> : std::integral_constant<size_t, Extent>
    {};

    template<class T, size_t Extent>
    struct extent_constant_impl<T[Extent]> : std::integral_constant<size_t, Extent>
    {};

    template<class C>
    struct extent_constant : extent_constant_impl<std::remove_cvref_t<C>>
    {};

    template<class C>
    constexpr auto extent_v = extent_constant<C>::value;
}

using private_detail_extent_constant::extent_constant;
using private_detail_extent_constant::extent_v;

namespace private_detail_compatible2span
{
    template<class C, size_t Extent>
    struct extent_compatible_impl
    {
        using type = std::bool_constant<Extent == extent_v<C>>;
    };

    template<class C>
    struct extent_compatible_impl<C, dynamic_extent>
    {
        using type = std::true_type;
    };

    template<class C, size_t Extent>
    using extent_compatible = typename extent_compatible_impl<C, Extent>::type;
}

template <class T, size_t Extent, class C>
constexpr bool is_compatible2span_v = std::conjunction_v
<
    std::negation<is_span<C>>,
    is_container<C>,
    is_convertible_data<C, T*>,
    private_detail_compatible2span::extent_compatible<C, Extent>
>;


template <class T, size_t Extent>
struct span_data_impl
{
    using pointer = T*;
    pointer data_{ nullptr };
    static constexpr size_t size_ = Extent;

    constexpr span_data_impl() noexcept = default;

    constexpr span_data_impl(pointer data, size_t) noexcept
        : data_{ data }
    {}
};


template <class T>
struct span_data_impl<T, dynamic_extent>
{
    using pointer = T*;
    pointer data_{ nullptr };
    size_t size_{ 0u };

    constexpr span_data_impl() noexcept = default;

    constexpr span_data_impl(pointer data, size_t size) noexcept
        : data_{ data }
        , size_{ size }
    {}
};

template <class T0, size_t E0, class T1, size_t E1>
constexpr bool is_compatible_span2span_v = std::conjunction_v
<
    std::is_convertible<T1*, T0*>,
    std::bool_constant<E0 == dynamic_extent || E0 == E1>
>;


template <class T, size_t Extent = dynamic_extent>
class span : private span_data_impl<T, Extent>
{
    using base_type = span_data_impl<T, Extent>;
    using base_type::data_;
    using base_type::size_;

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

    static constexpr size_type extent = Extent;

    template<class C>
    static constexpr bool is_compatible_v = is_compatible2span_v<T, Extent, C>;

    template<class OtherT, size_t OtherE>
    static constexpr bool is_compatible_span_v = is_compatible_span2span_v<T, Extent, OtherT, OtherE>;

    constexpr span() noexcept = default;

    constexpr span(pointer data, size_type size) noexcept
        : base_type{ data, size }
    {}

    constexpr span(const span&) noexcept = default;

    template<class OtherT, size_t OtherE, std::enable_if_t<is_compatible_span_v<OtherT, OtherE>, int> = 0>
    constexpr span(span<OtherT, OtherE> span) noexcept
        : base_type{ static_cast<pointer>(span.data()), span.size() }
    {}

    template<class C, std::enable_if_t<is_compatible_v<C>, int> = 0>
    constexpr span(C& c) noexcept
        : base_type{ std::data(c), narrow<size_type>(std::size(c)) }
    {}

    constexpr span& operator = (const span&) noexcept = default;

    template<class C>
    constexpr std::enable_if_t<is_compatible_v<C>, span&> operator = (C& container) noexcept
    {
        _base_ref() = base_type{ std::data(container), std::size(container) };
        return *this;
    }

    template<class OtherT, size_t OtherE>
    constexpr std::enable_if_t<is_compatible_span_v<OtherT, OtherE>, span&> operator = (span<OtherT, OtherE> span) noexcept
    {
        _base_ref() = base_type{ span.data(), span.size() };
        return *this;
    }

    [[nodiscard]]
    constexpr iterator begin() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr iterator end() const noexcept
    {
        return data_ + size_;
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return end();
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
    constexpr reference value(size_type index) const noexcept
    {
        return data_[index];
    }

    [[nodiscard]]
    constexpr reference operator[](size_type index) const noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr reference front() const noexcept
    {
        return value(0u);
    }

    [[nodiscard]]
    constexpr reference back() const noexcept
    {
        return value(size_ - 1u);
    }

    [[nodiscard]]
    constexpr span first(size_type size) const noexcept
    {
        return { data_, size };
    }

    [[nodiscard]]
    constexpr span subspan(size_type pos, size_type size) const noexcept
    {
        return { data_ + pos, size };
    }

    [[nodiscard]]
    constexpr span subspan(size_type pos) const noexcept
    {
        return { data_ + pos, size_ - pos };
    }

    [[nodiscard]]
    constexpr span last(size_type size) const noexcept
    {
        return { data_ + size_ - size, size };
    }

private:
    [[nodiscard]]
    constexpr base_type& _base_ref() noexcept
    {
        return *this;
    }
};

template <class Rng>
span(Rng&)->span<value_type_t<Rng>, extent_v<Rng>>;

template <class Rng>
span(const Rng&)->span<const value_type_t<Rng>, extent_v<Rng>>;
