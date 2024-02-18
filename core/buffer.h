#pragma once

#include <cstdlib>

#include <core/span.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_avoid_malloc_and_free)

struct buffer_construct_t
{};

constexpr buffer_construct_t buffer_construct{};

struct buffer_attach_construct_t
{};

constexpr buffer_attach_construct_t buffer_attach_construct{};

class buffer_void
{
public:
    using size_type = size_t;
    using null_type = nullmem_t;

    constexpr buffer_void() noexcept = default;

    D_DISABLE_COPY_CA(buffer_void);

    constexpr buffer_void(buffer_attach_construct_t, void* mem, size_type count) noexcept
        : data_{ mem }
        , count_{ (mem) ? count : 0_uz }
    {}

    constexpr buffer_void(buffer_void&& right) noexcept
        : data_{ std::exchange(right.data_, nullptr) }
        , count_{ std::exchange(right.count_, 0_uz) }
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
        free(data_);
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !!count_;
    }

    constexpr void swap(buffer_void& right) noexcept
    {
        std::swap(data_, right.data_);
        std::swap(count_, right.count_);
    }

    void reset() noexcept
    {
        [[maybe_unused]]
        const buffer_void temp{ std::move(*this) };
    }

    [[nodiscard]]
    constexpr const void* cvoid_data() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const void* void_data() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr void* void_data() noexcept
    {
        return data_;
    }

protected:
    [[nodiscard]]
    constexpr void* _void_data() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr size_type _count() const noexcept
    {
        return count_;
    }

private:
    void* data_{ nullptr };
    size_type count_{ 0_uz };
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

    buffer_void_collection(buffer_construct_t, size_type count) noexcept
        : base_type{ buffer_attach_construct, _alloc(count), count }
    {}

    constexpr buffer_void_collection(null_type nullvalue) noexcept
        : base_type{ nullvalue }
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

    [[nodiscard]] bool try_reserve(size_type new_count) noexcept
    {
        if (size() < new_count)
        {
            buffer_void_collection new_buffer{ buffer_construct, new_count };
            if (!new_buffer) [[unlikely]]
            {
                return false;
            }

            swap(new_buffer);
        }

        return true;
    }

    void reserve(size_type new_count) noexcept
    {
        D_ASSERT_OR_UNUSED(try_reserve(new_count));
    }

private:
    template<class T>
    [[nodiscard]] constexpr span<T> _as_span() const noexcept
    {
        return
        {
            static_cast<T*>(base_type::_void_data()),
            _count_for<T>()
        };
    }

    template<class T>
    [[nodiscard]] constexpr size_t _count_for() const noexcept
    {
        static_assert(!std::is_reference_v<T>);
        constexpr auto new_element_size = sizeof(T);

        if constexpr (new_element_size > element_size)
        {
            static_assert(!(new_element_size % element_size));
            constexpr auto new_interpretable_element_size = new_element_size / element_size;
            return size() / new_interpretable_element_size;
        }
        else
        {
            if constexpr (element_size == new_element_size)
            {
                return size();
            }
            else
            {
                static_assert(!(element_size % new_element_size));
                constexpr auto new_interpretable_element_size = element_size / new_element_size;
                return size_mul<new_interpretable_element_size>(size());
            }
        }
    }

    [[nodiscard]]
    static void* _alloc(size_type size) noexcept
    {
        void* result{ nullptr };

        if (has_size_mul<element_size>(size)) [[likely]]
        {
            using std::malloc;
            result = malloc(element_size * size);
        }

        return result;
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

    buffer(buffer_construct_t, size_t size) noexcept
        : base_type{ buffer_construct, size }
    {}

    constexpr buffer(null_type nullvalue) noexcept
        : base_type{ nullvalue }
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

static_assert(1_uz == sizeof(buffer_t::value_type));
static_assert(1_uz == buffer_t::element_size);

D_WARNING_POP