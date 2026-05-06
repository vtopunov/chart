#pragma once

#include <algorithm>
#include <array>

#include <core/utility.h>


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


namespace private_detail_span
{
    namespace private_detail_extent_constant
    {
        template<class C>
        using decl_extent_constant_t = index_constant<C::extent>;

        template<class C>
        struct extent_constant_for_impl : detected_or_t<index_constant<dynamic_extent>, decl_extent_constant_t, C>
        {};

        template<class T, size_t Extent>
        struct extent_constant_for_impl<std::array<T, Extent>> : index_constant<Extent>
        {};

        template<class T, size_t Extent>
        struct extent_constant_for_impl<span<T, Extent>> : index_constant<Extent>
        {};

        template<class T, size_t Extent>
        struct extent_constant_for_impl<T[Extent]> : index_constant<Extent>
        {};

        template<class C>
        using extent_constant_for = extent_constant_for_impl<std::remove_cvref_t<C>>;

        template<class C>
        constexpr auto extent_v = extent_constant_for<C>::value;
    }
}

using private_detail_span::private_detail_extent_constant::extent_constant_for;
using private_detail_span::private_detail_extent_constant::extent_v;


namespace private_detail_span
{
    namespace private_detail_has_extent_compatible
    {
        template<class Container, size_t Extent>
        struct has_extent_compatible_impl
        {
            using type = is_less_equal_size<Extent, extent_v<Container> >;
        };

        template<class Container>
        struct has_extent_compatible_impl<Container, dynamic_extent>
        {
            using type = std::true_type;
        };

        template<size_t Extent, class Container>
        using has_extent_compatible = typename has_extent_compatible_impl<Container, Extent>::type;
    }
}

using private_detail_span::private_detail_has_extent_compatible::has_extent_compatible;

template <class Target, size_t Extent, class Container>
constexpr bool is_compatible2span_v = std::conjunction_v
<
    std::negation<is_span<Container>>,
    has_std_size<Container>,
    has_std_data_compatible<Target, Container>,
    has_extent_compatible<Extent, Container>
>;

template <class T, size_t E, class OtherT, size_t OtherE>
constexpr bool is_compatible_span2span_v = std::conjunction_v
<
    std::disjunction<std::negation<is_same_is_const<T, OtherT>>, is_nsame_size<E, OtherE> >,
    is_const_convertible<OtherT, T>,
    std::disjunction<is_same_size<E, dynamic_extent>, is_less_equal_size<E, OtherE> >
>;


template <class T, size_t Extent>
struct span_data_impl
{
    using pointer = T*;
    pointer data_{ nullptr };
    static constexpr size_t size_ = Extent;

    D_DEFAULT_ALL_CAEQ(span_data_impl);

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
    using pointer = value_type*;
    using const_pointer = const_value_type*;
    using reference = value_type&;
    using const_reference = const_value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using const_span_type = span<const_value_type, Extent>;
    using dynamic_extent_span_type = span<value_type, dynamic_extent>;

    using view_type = const_span_type;
    using null_type = nullmem_t;

    static constexpr size_type extent = Extent;

    template<class C>
    static constexpr bool is_compatible_v = is_compatible2span_v<T, Extent, std::remove_reference_t<C>>;

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
        : base_type{ span.data(), span.size() }
    {}

    template<class C, std::enable_if_t<is_compatible_v<C>, int> = 0>
    constexpr span(C&& c) noexcept
        : base_type{ std::data(c), narrow<size_type>(std::size(c)) }
    {}


    template<class C, std::enable_if_t<is_compatible_v<const C>, int> = 0>
    constexpr span(const C& c) noexcept
        : base_type{ std::data(c), narrow<size_type>(std::size(c)) }
    {}

    constexpr span& operator = (null_type null_value) noexcept
    {
        return operator=(static_cast<span>(null_value));
    }

    template<class C>
    constexpr std::enable_if_t<is_compatible_v<C>, span&> operator = (C&& container) noexcept
    {
        _base_ref() = base_type{ std::data(container), narrow<size_type>(std::size(container)) };
        return *this;
    }

    template<class C>
    constexpr std::enable_if_t<is_compatible_v<const C>, span&> operator = (const C& container) noexcept
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
        return cdata();
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return cdata() + size_;
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]]
    constexpr const_pointer cdata() const noexcept
    {
        return data_;
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
        D_ASSERT(0u < size_);
        return *data_;
    }

    [[nodiscard]]
    constexpr reference back() const noexcept
    {
        D_ASSERT(0u < size_);
        return data_[size_ - 1u];
    }

    [[nodiscard]]
    constexpr dynamic_extent_span_type first(size_type size) const noexcept
    {
        D_ASSERT(size <= size_);
        return { data_, size };
    }

    [[nodiscard]]
    constexpr dynamic_extent_span_type subspan(size_type pos, size_type size) const noexcept
    {
        D_ASSERT(pos <= size_);
        D_ASSERT(size <= (size_ - pos));
        return { data_ + pos, size };
    }

    [[nodiscard]]
    constexpr dynamic_extent_span_type subspan(size_type pos) const noexcept
    {
        D_ASSERT(pos <= size_);
        return { data_ + pos, size_ - pos };
    }

    [[nodiscard]]
    constexpr dynamic_extent_span_type last(size_type size) const noexcept
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
span(Rng&&) -> span<value_type_t<Rng>, extent_v<Rng>>;

template <class Rng>
span(const Rng&) -> span<const value_type_t<Rng>, extent_v<Rng>>;


namespace private_detail_span
{
    namespace private_detail_make_span
    {
        template<class Rng>
        struct make_span_type_helper
        {
            using value_type = value_type_t<Rng>;
            using const_value_type = std::add_const_t<value_type>;
            static constexpr size_t extent = extent_v<Rng>;
            using span_type = span<value_type, extent>;
            using const_span_type = span<const_value_type, extent>;
        };

        template<class Rng>
        struct make_span_type
        {
            using type = typename make_span_type_helper<std::remove_reference_t<Rng>>::span_type;
        };

        template<class Rng>
        struct make_cspan_type
        {
            using type = typename make_span_type_helper<std::remove_reference_t<Rng>>::const_span_type;
        };

        template<class Rng>
        using make_span_t = typename make_span_type<Rng>::type;

        template<class Rng>
        using make_cspan_t = typename make_cspan_type<Rng>::type;
    }
}

using private_detail_span::private_detail_make_span::make_span_t;
using private_detail_span::private_detail_make_span::make_cspan_t;

template<class Rng>
[[nodiscard]] constexpr make_span_t<Rng> make_span(Rng&& rng) noexcept
{
    return rng;
}

template<class Rng>
[[nodiscard]] constexpr make_cspan_t<const Rng> make_cspan(const Rng& rng) noexcept
{
    return rng;
}

template<class OutT, size_t Extent, class T>
constexpr void fill(span<OutT, Extent> sp, const T& value) noexcept
{
    std::fill_n(sp.begin(), sp.size(), value);
}

template<class Rng, class T>
constexpr auto fill(Rng&& rng, const T& value) noexcept -> decltype
(
    fill(make_span(std::forward<Rng>(rng)), value)
)
{
    fill(make_span(std::forward<Rng>(rng)), value);
}

template<class InT, size_t Extent, class OutIt>
constexpr OutIt copy(span<const InT, Extent> sp, OutIt out) noexcept
{
    return std::copy_n(sp.cbegin(), sp.size(), out);
}

template<class Rng, class OutIt>
constexpr auto copy(const Rng& rng, OutIt out) noexcept -> decltype(copy(make_cspan(rng), out))
{
    return copy(make_cspan(rng), out);
}

template<class SpanValueT, size_t Extent, class T>
[[nodiscard]] constexpr size_t find_n(const span<const SpanValueT, Extent> sp, const T& value, size_t pos = 0u) noexcept
{
    D_ASSERT(pos <= sp.size());

    for (; pos != sp.size(); ++pos)
    {
        if (value == sp[pos])
            break;
    }

    return pos;
}

template<class Rng, class T>
[[nodiscard]] constexpr auto find_n(const Rng& rng, const T& value, size_t pos = 0u) noexcept -> decltype
(
    find_n(make_cspan(rng), value, pos)
)
{
    return find_n(make_cspan(rng), value, pos);
}