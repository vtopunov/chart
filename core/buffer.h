#pragma once

#include <cstdlib>

#include <core/span.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_avoid_malloc_and_free)


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
        using std::free;
        free(mem_.data);
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !!mem_.count;
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
    constexpr const void* void_data() const noexcept
    {
        return mem_.data;
    }

    [[nodiscard]]
    constexpr void* void_data() noexcept
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
    constexpr void* _void_data() const noexcept
    {
        return mem_.data;
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
    static constexpr auto element_size = ElementSize;

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
    [[nodiscard]] constexpr span<const T> as_span() const noexcept
    {
        return _as_span<const T>();
    }

    template<class T>
    [[nodiscard]] constexpr span<T> as_span() noexcept
    {
        return _as_span<T>();
    }

    [[nodiscard]] constexpr span<const std::byte> as_bytes() const noexcept
    {
        return _as_span<const std::byte>();
    }

    [[nodiscard]] constexpr span<std::byte> as_bytes() noexcept
    {
        return _as_span<std::byte>();
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

private:
    template<class T>
    [[nodiscard]] constexpr span<T> _as_span() const noexcept
    {
        static_assert(!std::is_reference_v<T>);
        return
        {
            static_cast<T*>(base_type::_void_data()),
            _count_for<sizeof(T)>(size())
        };
    }

    template<size_t NewElementSize>
    [[nodiscard]] static constexpr size_t _count_for(size_t size) noexcept
    {
        static_assert(std::is_same_v<size_type, size_t>);

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
        static_assert(std::is_same_v<size_type, size_t>);

        if (has_size_mul<element_size>(count)) [[likely]]
        {
            using std::malloc;

            if (const auto data = malloc(element_size * count)) [[likely]]
            {
                return { .data{ data }, .count{ count } };
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
    constexpr const_pointer data() const noexcept
    {
        return static_cast<const_pointer>(buffer_void::cvoid_data());
    }

    [[nodiscard]]
    constexpr pointer data() noexcept
    {
        return static_cast<pointer>(buffer_void::void_data());
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return data();
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return data() + size();
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept
    {
        return cbegin();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept
    {
        return cend();
    }

    [[nodiscard]]
    constexpr iterator begin() noexcept
    {
        return data();
    }

    [[nodiscard]]
    constexpr iterator end() noexcept
    {
        return const_cast<iterator>(cend());
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        return *data();
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        return *(cend() - 1_uz);
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept
    {
        return cfront();
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept
    {
        return cback();
    }

    [[nodiscard]]
    constexpr reference front() noexcept
    {
        return const_cast<reference>(cfront());
    }

    [[nodiscard]]
    constexpr reference back() noexcept
    {
        return const_cast<reference>(cback());
    }

    [[nodiscard]]
    constexpr const_reference operator[](size_type index) const noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr reference operator[](size_type index) noexcept
    {
        return value(index);
    }

    [[nodiscard]]
    constexpr const_reference cvalue(size_type index) const noexcept
    {
        return data()[index];
    }

    [[nodiscard]]
    constexpr const_reference value(size_type index) const noexcept
    {
        return cvalue(index);
    }

    [[nodiscard]]
    constexpr reference value(size_type index) noexcept
    {
        return const_cast<reference>(cvalue(index));
    }

    [[nodiscard]] constexpr span<const_value_type> as_span() const noexcept
    {
        return base_type::template as_span<const_value_type>();
    }

    [[nodiscard]] constexpr span<value_type> as_span() noexcept
    {
        return base_type::template as_span<value_type>();
    }
};

static_assert(!std::is_copy_constructible_v<byte_buffer>);
static_assert(!std::is_copy_assignable_v<byte_buffer>);

D_WARNING_POP