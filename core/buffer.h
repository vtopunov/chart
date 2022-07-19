#pragma once

#include <cstdlib>

#include <core/buffer_fwd.h>
#include <core/warnings.h>
#include <core/utility.h>
#include <core/size_type.h>

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
    constexpr buffer_void() noexcept = default;
    
    D_DISABLE_COPY(buffer_void);

    constexpr buffer_void(buffer_attach_construct_t, void* mem, size_t count) noexcept
        : data_{ mem }
        , count_{ (mem) ? count : 0_uz }
    {}

    constexpr buffer_void(buffer_void&& right) noexcept
        : data_{ std::exchange(right.data_, nullptr) }
        , count_{ std::exchange(right.count_, 0_uz) }
    {}

    constexpr buffer_void& operator = (buffer_void&& right) noexcept
    {
        swap(right);
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
        return !!data_;
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
        return cvoid_data();
    }

    [[nodiscard]]
    constexpr void* void_data() noexcept
    {
        return data_;
    }

protected:
    [[nodiscard]]
    constexpr size_t _count() const noexcept
    {
        return count_;
    }

private:
    void* data_{ nullptr };
    size_t count_{ 0_uz };
};

template<size_t ElementSize>
class buffer_void_collection : public buffer_void
{
    using base_type = buffer_void;

public:
    static constexpr auto element_size = ElementSize;

    constexpr buffer_void_collection() noexcept = default;

    D_DEFAULT_MOVABLE_ONLY(buffer_void_collection);

    constexpr buffer_void_collection(buffer_attach_construct_t, void* mem, size_t count) noexcept
        : base_type{ buffer_attach_construct, mem, count }
    {}

    template<class T>
    constexpr buffer_void_collection(buffer_attach_construct_t, T* mem, size_t count) noexcept
        : base_type{ buffer_attach_construct, mem, count }
    {
        static_assert(element_size == sizeof(T));
    }

    buffer_void_collection(buffer_construct_t, size_t count) noexcept
        : base_type{ buffer_attach_construct, _alloc(count), count }
    {}

    template<class T>
    [[nodiscard]] constexpr const T* as_ptr() const noexcept
    {
        static_assert(_is_compatible_element_size(sizeof(T)));
        return static_cast<const T*>(cvoid_data());
    }

    template<class T>
    [[nodiscard]] constexpr T* as_ptr() noexcept
    {
        static_assert(_is_compatible_element_size(sizeof(T)));
        return static_cast<T*>(void_data());
    }

    [[nodiscard]] constexpr size_t size_bytes() const noexcept
    {
        return size_mul<element_size>(_count());
    }

    [[nodiscard]]
    bool try_reserve(size_t new_count) noexcept
    {
        if (_count() < new_count)
        {
            buffer_void_collection new_buffer{ buffer_construct, new_count };
            if (!new_buffer)
            {
                return false;
            }

            swap(new_buffer);
        }

        return true;
    }

private:
    [[nodiscard]]
    static constexpr bool _is_compatible_element_size(size_t testing_size) noexcept
    {
        return (testing_size >= element_size) && !(testing_size % element_size);
    }

    static void* _alloc(size_t size) noexcept
    {
        constexpr size_t overflow = numeric_max_v<size_t> / element_size;

        void* result{ nullptr };

        if (size <= overflow)
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
    using size_type = size_t;
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;

    constexpr buffer() noexcept = default;

    D_DEFAULT_MOVABLE_ONLY(buffer);

    constexpr buffer(buffer_attach_construct_t, pointer mem, size_t size) noexcept
        : base_type{ buffer_attach_construct, mem, size }
    {}

    buffer(buffer_construct_t, size_t size) noexcept
        : base_type{ buffer_construct, size }
    {}

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return buffer_void::_count();
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
        return _end();
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
        return const_cast<pointer>(_end());
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        return *data();
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        return *(_end() - 1_uz);
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

private:
    [[nodiscard]]
    constexpr const_pointer _end() const noexcept
    {
        return data() + size();
    }
};

static_assert(1_uz == sizeof(buffer_t::value_type));

D_WARNING_POP