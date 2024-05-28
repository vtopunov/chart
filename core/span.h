#pragma once

#include <algorithm>
#include <array>

#include <core/utility.h>
#include <core/narrow.h>


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


namespace private_detail_extent_constant
{
    template<size_t Extent>
    using extent_integral_constant = std::integral_constant<size_t, Extent>;

    template<class C>
    using decl_extent_constant_t = extent_integral_constant<C::extent>;

    template<class C>
    struct extent_constant_for_impl : detected_or_t<extent_integral_constant<dynamic_extent>, decl_extent_constant_t, C>
    {};

    template<class T, size_t Extent>
    struct extent_constant_for_impl<std::array<T, Extent>> : extent_integral_constant<Extent>
    {};

    template<class T, size_t Extent>
    struct extent_constant_for_impl<span<T, Extent>> : extent_integral_constant<Extent>
    {};

    template<class T, size_t Extent>
    struct extent_constant_for_impl<T[Extent]> : extent_integral_constant<Extent>
    {};

    template<class C>
    using extent_constant_for = extent_constant_for_impl<std::remove_cvref_t<C>>;

    template<class C>
    constexpr auto extent_v = extent_constant_for<C>::value;
}

using private_detail_extent_constant::extent_constant_for;
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
    has_std_size<C>,
    is_std_data_convertible<C, T*>,
    private_detail_compatible2span::extent_compatible<C, Extent>
>;

template <class T0, size_t E0, class T1, size_t E1>
constexpr bool is_compatible_span2span_v = std::conjunction_v
<
    std::is_convertible<T1*, T0*>,
    std::bool_constant<E0 == dynamic_extent || E0 == E1>
>;


template <class T, size_t Extent>
struct span_data_impl
{
    using pointer = T*;
    pointer data_{ nullptr };
    static constexpr size_t size_ = Extent;

    D_DEFAULT_ALL_CAEQ(span_data_impl);

    constexpr span_data_impl(pointer data, [[maybe_unused]] size_t size) noexcept
        : data_{ data }
    {
        D_ASSERT(size_ == size);
    }
};

template <class T>
struct span_data_impl<T, dynamic_extent>
{
    using pointer = T*;
    pointer data_{ nullptr };
    size_t size_{ 0u };

    D_DEFAULT_ALL_CAEQ(span_data_impl);

    constexpr span_data_impl(pointer data, size_t size) noexcept
        : data_{ data }
        , size_{ size }
    {}
};

template <class T, size_t Extent>
class span : private span_data_impl<T, Extent>
{
    using base_type = span_data_impl<T, Extent>;
    using base_type::data_;
    using base_type::size_;

public:
    using value_type = T;
    using const_value_type = const value_type;
    using pointer = T*;
    using const_pointer = const_value_type*;
    using reference = T&;
    using const_reference = const_value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using view_type = span<const_value_type, Extent>;
    using null_type = nullmem_t;

    static constexpr size_type extent = Extent;

    template<class C>
    static constexpr bool is_compatible_v = is_compatible2span_v<T, Extent, C>;

    template<class OtherT, size_t OtherE>
    static constexpr bool is_compatible_span_v = is_compatible_span2span_v<T, Extent, OtherT, OtherE>;

    D_DEFAULT_ALL_CAEQ(span);

    constexpr span(null_type) noexcept
        : span{}
    {}

    constexpr span(pointer data, size_type size) noexcept
        : base_type{ data, size }
    {}

    template<class OtherT, size_t OtherE, std::enable_if_t<is_compatible_span_v<OtherT, OtherE>, int> = 0>
    constexpr span(span<OtherT, OtherE> span) noexcept
        : base_type{ static_cast<pointer>(span.data()), span.size() }
    {}

    template<class C, std::enable_if_t<is_compatible_v<std::remove_reference_t<C>>, int> = 0>
    constexpr span(C&& c) noexcept
        : base_type{ std::data(c), narrow<size_type>(std::size(c)) }
    {}

    constexpr span& operator = (null_type null_value) noexcept
    {
        return operator=(static_cast<span>(null_value));
    }

    template<class C>
    constexpr std::enable_if_t<is_compatible_v<C>, span&> operator = (C& container) noexcept
    {
        _base_ref() = base_type{ std::data(container), narrow<size_type>(std::size(container)) };
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
        D_ASSERT(index < size_);
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
        D_ASSERT(size <= size_);
        return { data_, size };
    }

    [[nodiscard]]
    constexpr span subspan(size_type pos, size_type size) const noexcept
    {
        D_ASSERT(pos <= size_);
        D_ASSERT(size <= (size_ - pos));
        return { data_ + pos, size };
    }

    [[nodiscard]]
    constexpr span subspan(size_type pos) const noexcept
    {
        D_ASSERT(pos <= size_);
        return { data_ + pos, size_ - pos };
    }

    [[nodiscard]]
    constexpr span last(size_type size) const noexcept
    {
        D_ASSERT(size <= size_);
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
span(Rng&) -> span<value_type_t<Rng>, extent_v<Rng>>;

template <class Rng>
span(const Rng&) -> span<const value_type_t<Rng>, extent_v<Rng>>;


template<class OutT, size_t Extent, class T>
constexpr void fill(span<OutT, Extent> sp, const T& value) noexcept
{
    std::fill_n(sp.data(), sp.size(), value);
}

template<class InT, size_t Extent, class OutIt>
constexpr OutIt copy(span<InT, Extent> sp, OutIt out) noexcept
{
    return std::copy_n(sp.data(), sp.size(), out);
}
