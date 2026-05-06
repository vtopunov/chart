#pragma once

#include <mimalloc.h>

#include <core/buffer_view.h>


class buffer_void
{
public:
    using size_type = size_t;
    using null_type = nullmem_t;

    constexpr buffer_void() noexcept = default;

    D_DISABLE_COPY_CA(buffer_void);

    constexpr buffer_void(buffer_void&& right) noexcept
        : mem_{ _release(right.mem_) }
    {}

    constexpr buffer_void(null_type) noexcept
        : buffer_void{}
    {}

    constexpr buffer_void& operator = (buffer_void&& right) noexcept
    {
        swap(right);
        return *this;
    }

    buffer_void& operator = (null_type) noexcept
    {
        reset();
        return *this;
    }

    ~buffer_void() noexcept
    {
        mi_free(mem_.data);
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !is_null_or_empty();
    }

    [[nodiscard]]
    constexpr bool is_null_or_empty() const noexcept
    {
        return !mem_.count;
    }

    constexpr void swap(buffer_void& right) noexcept
    {
        std::swap(mem_, right.mem_);
    }

    void reset() noexcept
    {
        [[maybe_unused]]
        const buffer_void temp{ std::move(*this) };
    }

    [[nodiscard]]
    constexpr const void* cvoid_data() const noexcept
    {
        return mem_.data;
    }

    [[nodiscard]]
    constexpr void* void_data() const noexcept
    {
        return mem_.data;
    }

protected:
    struct memory_location
    {
        void* data;
        size_type count;
    };

    static constexpr memory_location nullmem_location{ .data{ nullptr }, .count{ 0_uz } };

    constexpr explicit buffer_void(memory_location mem) noexcept
        : mem_{ mem }
    {
        D_ASSERT(!(mem.data) == !(mem.count));
    }

    [[nodiscard]]
    constexpr size_type _count() const noexcept
    {
        return mem_.count;
    }

private:
    [[nodiscard]]
    static constexpr memory_location _release(memory_location& mem) noexcept
    {
        const auto temp = mem;
        mem = nullmem_location;
        return temp;
    }

private:
    memory_location mem_{ nullmem_location };
};

template<size_t ElementSize>
class buffer_void_collection : public buffer_void
{
    using base_type = buffer_void;

public:
    using typename base_type::size_type;
    using typename base_type::null_type;
    static_assert(std::is_same_v<size_type, size_t>);

    static constexpr auto element_size = ElementSize;
    static_assert(0_uz < element_size);


    constexpr buffer_void_collection() noexcept = default;

    D_DEFAULT_ONLYMOVE_CA(buffer_void_collection);

    constexpr buffer_void_collection(null_type nullvalue) noexcept
        : base_type{ nullvalue }
    {}

    constexpr explicit buffer_void_collection(size_type count) noexcept
        : base_type{ _alloc(count) }
    {}

    buffer_void_collection& operator = (null_type nullvalue) noexcept
    {
        base_type::operator=(nullvalue);
        return *this;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return buffer_void::_count();
    }

    [[nodiscard]] constexpr size_type size_bytes() const noexcept
    {
        return size_mul<element_size>(size());
    }

    [[nodiscard]] constexpr bool try_reserve(size_type new_count) noexcept
    {
        if (size() < new_count)
        {
            buffer_void_collection new_buffer{ new_count };
            if (!new_buffer) [[unlikely]]
            {
                return false;
            }

            swap(new_buffer);
        }

        return true;
    }

    constexpr void reserve(size_type new_count) noexcept
    {
        D_CHECK(try_reserve(new_count));
    }

    [[nodiscard]]
    static size_type good_size(size_type size) noexcept
    {
        if (has_size_mul<element_size>(size)) [[likely]]
        {
            const auto good_size = mi_good_size(element_size * size) / element_size;
            D_ASSERT(size <= good_size);
            return size;
        }

        return size;
    }

private:
    [[nodiscard]]
    static constexpr memory_location _alloc(size_t count) noexcept
    {
        if (has_size_mul<element_size>(count)) [[likely]]
        {
            if (const auto data = mi_malloc(element_size * count)) [[likely]]
            {
                const auto usable_count = mi_usable_size(data) / element_size;
                D_ASSERT(count <= usable_count);
                return { .data{ data }, .count{ usable_count } };
            }
        }

        return nullmem_location;
    }
};

template<class T>
class buffer : public buffer_void_collection<sizeof(T)>
{
public:
    using collection_type = buffer_void_collection<sizeof(T)>;
    using value_type = T;
    using const_value_type = const value_type;
    using pointer = value_type*;
    using const_pointer = const_value_type*;
    using reference = value_type&;
    using const_reference = const_value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using view_type = basic_buffer_view<value_type>;
    using const_view_type = basic_buffer_view<const_value_type>;
    using typename collection_type::size_type;
    using typename collection_type::null_type;
    using collection_type::size;

    constexpr buffer() noexcept = default;

    D_DEFAULT_ONLYMOVE_CA(buffer);

    constexpr buffer(null_type nullvalue) noexcept
        : collection_type{ nullvalue }
    {}

    constexpr explicit buffer(size_type count) noexcept
        : collection_type{ count }
    {}

    buffer& operator = (null_type nullvalue) noexcept
    {
        collection_type::operator=(nullvalue);
        return *this;
    }

    [[nodiscard]]
    constexpr const_pointer cdata() const noexcept
    {
        return static_cast<const_pointer>(buffer_void::cvoid_data());
    }

    [[nodiscard]]
    constexpr pointer data() const noexcept
    {
        return const_cast<pointer>(cdata());
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return cdata();
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return cdata() + size();
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
    constexpr const_reference cfront() const noexcept
    {
        D_ASSERT(0u < size());
        return *cdata();
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        D_ASSERT(0u < size());
        return cdata()[size() - 1u];
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
    constexpr reference operator[](size_type index) const noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr const_reference cvalue(size_type index) const noexcept
    {
        D_ASSERT(index < size());
        return cdata()[index];
    }

    [[nodiscard]]
    constexpr reference value(size_type index) const noexcept
    {
        return const_cast<reference>(cvalue(index));
    }
};