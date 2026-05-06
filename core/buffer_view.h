#pragma once

#include <core/utility.h>


namespace private_detail_buffer_view
{
    template<class C>
    using decl_void_data_t = decltype(std::declval<C&>().void_data());

    template<class C>
    using has_void_data = is_detected<decl_void_data_t, C>;

    template <bool immutable, class Container>
    using is_compatible_impl = std::conjunction<
        has_size_bytes<Container>,
        has_std_data_void_compatible<immutable, Container>
    >;

    template <bool immutable, class Container>
    constexpr bool is_compatible_impl_v = is_compatible_impl<immutable, Container>::value;

    template <bool immutable, class Container>
    constexpr bool is_compatible_container_impl_v = std::conjunction_v<
        std::negation<has_void_data<Container>>,
        is_compatible_impl<immutable, Container>
    >;

    template <bool immutable, class Container>
    constexpr bool is_compatible_buffer_impl_v = std::conjunction_v<
        has_void_data<Container>,
        is_compatible_impl<immutable, Container>
    >;
}

template<class T>
class basic_buffer_view
{
public:
    static constexpr bool immutable = std::is_const_v<T>;

    using value_type = T;
    using const_value_type = const value_type;
    using size_type = size_t;
    using pointer = value_type*;
    using const_pointer = const_value_type*;
    using reference = value_type&;
    using const_reference = const_value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using view_type = basic_buffer_view<const_value_type>;
    using null_type = nullmem_t;
    using void_pointer = std::add_pointer_t<conditional_add_const_t<immutable, void>>;
    using const_void_pointer = const void*;

    template<class C>
    static constexpr bool is_compatible_v = private_detail_buffer_view::is_compatible_impl_v<immutable, C>;

    template<class C>
    static constexpr bool is_compatible_container_v = private_detail_buffer_view::is_compatible_container_impl_v<immutable, C>;

    template<class C>
    static constexpr bool is_compatible_buffer_v = private_detail_buffer_view::is_compatible_buffer_impl_v<immutable, C>;

    D_DEFAULT_ALL_CAEQ(basic_buffer_view);

    constexpr basic_buffer_view(null_type) noexcept
        : basic_buffer_view{}
    {}

    template<size_t OtherElementSize>
    constexpr basic_buffer_view
    (
        memory_construct_t,
        index_constant<OtherElementSize>,
        void_pointer data,
        size_t size
    ) noexcept
        : data_{ data }
        , size_{ reinterpret_size<OtherElementSize, sizeof(value_type)>(size) }
    {
        D_ASSERT(!(data_) == !(size_));
    }

    template<class OtherT>
    constexpr basic_buffer_view(memory_construct_t, OtherT* data, size_type size) noexcept
        : basic_buffer_view{ memory_construct, index_constant_v<sizeof_v<OtherT>>, data, size }
    {}

    template<class C, std::enable_if_t<is_compatible_container_v<C>, int> = 0>
    constexpr basic_buffer_view(C&& container) noexcept
        : basic_buffer_view{ memory_construct, std::data(container), narrow<size_type>(std::size(container)) }
    {}

    template<class C, std::enable_if_t<is_compatible_container_v<const C>, int> = 0>
    constexpr basic_buffer_view(const C& container) noexcept
        : basic_buffer_view{ memory_construct, std::data(container), narrow<size_type>(std::size(container)) }
    {}

    template<class C, std::enable_if_t<is_compatible_buffer_v<C>, int> = 0>
    constexpr basic_buffer_view(C&& container) noexcept
        : basic_buffer_view
        {
            memory_construct,
            index_constant_v<sizeof_v<value_type_t<C>>>,
            container.void_data(),
            narrow<size_t>(std::size(container))
        }
    {}

    template<class C, std::enable_if_t<is_compatible_buffer_v<const C>, int> = 0>
    constexpr basic_buffer_view(const C& container) noexcept
        : basic_buffer_view
        {
            memory_construct,
            index_constant_v<sizeof_v<decl_std_data_value_t<C>>>,
            container.void_data(),
            narrow<size_t>(std::size(container))
        }
    {}

    constexpr basic_buffer_view& operator = (null_type nullvalue) noexcept
    {
        return basic_buffer_view::operator=(basic_buffer_view(nullvalue));
    }

    template<class C>
    constexpr std::enable_if_t<is_compatible_v<C>, basic_buffer_view&> operator = (C&& container) noexcept
    {
        return operator = (basic_buffer_view(container));
    }

    template<class C>
    constexpr std::enable_if_t<is_compatible_v<const C>, basic_buffer_view&> operator = (const C& container) noexcept
    {
        return operator = (basic_buffer_view(container));
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !!size_;
    }

    [[nodiscard]]
    constexpr const_void_pointer cvoid_data() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr void_pointer void_data() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr pointer data() const noexcept
    {
        return const_cast<pointer>(cdata());
    }

    [[nodiscard]]
    constexpr const_pointer cdata() const noexcept
    {
        return static_cast<const_pointer>(cvoid_data());
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]]
    constexpr basic_buffer_view<const_value_type> as_const() const noexcept
    {
        return *this;
    }

    [[nodiscard]]
    constexpr reference value(size_type index) const noexcept
    {
        D_ASSERT(index < size());
        return data()[index];
    }

    [[nodiscard]]
    constexpr reference operator[](size_type index) const noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr reference front() const noexcept
    {
        return const_cast<reference>(cfront());
    }

    [[nodiscard]]
    constexpr reference back() const noexcept
    {
        return const_cast<reference>(cback());
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        D_ASSERT(0u < size());
        return *cdata();
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        D_ASSERT(0u < size_);
        return cdata()[size_ - 1u];
    }

    [[nodiscard]]
    constexpr iterator begin() const noexcept
    {
        return data();
    }

    [[nodiscard]]
    constexpr iterator end() const noexcept
    {
        return const_cast<iterator>(cend());
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

private:
    void_pointer data_{ nullptr };
    size_type size_{ 0u };
};

template<class T, class U>
[[nodiscard]] constexpr basic_buffer_view<copy_const_t<U, T>> interpret(basic_buffer_view<U> buffer) noexcept
{
    return buffer;
}

inline void zero_memory(byte_buffer_view buffer) noexcept
{
    memset(buffer.data(), 0, buffer.size());
}