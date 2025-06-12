#pragma once

#include <mimalloc.h>

#include <core/span.h>


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
    constexpr buffer_void(void* data, size_type count) noexcept
        : mem_{ data, count }
    {
        D_ASSERT(data || !count);
    }

    struct memory_location
    {
        void* data;
        size_type count;
    };

    static constexpr memory_location nullmem_location{ .data{ nullptr }, .count{ 0_uz } };

    constexpr explicit buffer_void(memory_location mem) noexcept
        : mem_{ mem }
    {}

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

    template<class T>
    [[nodiscard]] constexpr span<T> as_span() const noexcept
    {
        static_assert(!std::is_reference_v<T>);
        return
        {
            static_cast<T*>(base_type::void_data()),
            _count_for<sizeof(T)>(size())
        };
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
        D_ASSERT_OR_UNUSED(try_reserve(new_count));
    }

    [[nodiscard]]
    static size_type good_size(size_type size) noexcept
    {
        return mi_good_size(size_mul_or_max<element_size>(size)) / element_size;
    }

private:
    template<size_t NewElementSize>
    [[nodiscard]] static constexpr size_t _count_for(size_t size) noexcept
    {
        constexpr auto new_element_size = NewElementSize;

        if constexpr (new_element_size > element_size)
        {
            static_assert(!(new_element_size % element_size));
            constexpr auto new_interpretable_element_size = new_element_size / element_size;
            return size / new_interpretable_element_size;
        }
        else
        {
            if constexpr (element_size == new_element_size)
            {
                return size;
            }
            else
            {
                static_assert(!(element_size % new_element_size));
                constexpr auto new_interpretable_element_size = element_size / new_element_size;
                return size_mul<new_interpretable_element_size>(size);
            }
        }
    }

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
    using base_type = buffer_void_collection<sizeof(T)>;

public:
    using value_type = T;
    using const_value_type = const value_type;
    using pointer = value_type*;
    using const_pointer = const_value_type*;
    using reference = value_type&;
    using const_reference = const_value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using view_type = span<const_value_type>;
    using typename base_type::size_type;
    using typename base_type::null_type;
    using base_type::size;
    using base_type::as_span;

    constexpr buffer() noexcept = default;

    D_DEFAULT_ONLYMOVE_CA(buffer);

    constexpr buffer(null_type nullvalue) noexcept
        : base_type{ nullvalue }
    {}

    constexpr explicit buffer(size_type count) noexcept
        : base_type{ count }
    {}

    constexpr buffer(pointer mem, size_type size) noexcept
        : base_type{ mem, size }
    {}

    buffer& operator = (null_type nullvalue) noexcept
    {
        base_type::operator=(nullvalue);
        return *this;
    }

    [[nodiscard]]
    constexpr const_pointer cdata() const noexcept
    {
        return data();
    }

    [[nodiscard]]
    constexpr pointer data() const noexcept
    {
        return static_cast<pointer>(buffer_void::void_data());
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
    constexpr iterator begin() const noexcept
    {
        return data();
    }

    [[nodiscard]]
    constexpr iterator end() const noexcept
    {
        return data() + size();
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        return front();
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        return back();
    }

    [[nodiscard]]
    constexpr reference front() const noexcept
    {
        D_ASSERT(0_uz < size());
        return *data();
    }

    [[nodiscard]]
    constexpr reference back() const noexcept
    {
        D_ASSERT(0_uz < size());
        return *(end() - 1);
    }

    [[nodiscard]]
    constexpr reference operator[](size_type index) const noexcept
    {
        return data()[index];
    }

    [[nodiscard]]
    constexpr const_reference cvalue(size_type index) const noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr reference value(size_type index) const noexcept
    {
        D_ASSERT(index < size());
        return data()[index];
    }

    [[nodiscard]] constexpr span<value_type> as_span() const noexcept
    {
        return base_type::template as_span<value_type>();
    }

    [[nodiscard]] constexpr span<const_value_type> as_cspan() const noexcept
    {
        return base_type::template as_span<const_value_type>();
    }
};