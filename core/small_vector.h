#pragma once

#include <memory>
#include <algorithm>
#include <optional>

#include <core/uninitialized_dynarray.h>
#include <core/span.h>

#undef min
#undef max

constexpr size_t optimal_geometric_growth(size_t value) noexcept
{
    constexpr size_t num_fib{ 5u };
    constexpr size_t den_fib{ 8u };
    constexpr auto max_value = std::numeric_limits<size_t>::max();

    constexpr auto num_overflow = max_value / num_fib;

    const auto increment
        = (value <= num_overflow)
        ? (num_fib * value) / den_fib
        : num_fib * (value / den_fib);

    const auto increment_overflow = max_value - increment;

    const auto result
        = (value <= increment_overflow)
        ? value + increment
        : max_value;

    return result;
}

template<class T, size_t N>
class small_vector
{
    using self = small_vector;

public:
    static_assert(N > 0);
    static constexpr size_t small_size = N;

    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using span_type = span<value_type>;
    using const_span_type = span<std::add_const_t<value_type>>;
    using uninitialized_dynarray_type = uninitialized_dynarray<value_type>;
    using size_type = typename uninitialized_dynarray_type::size_type;

    constexpr small_vector() noexcept
        : data_{ small_ }
        , size_{ 0u }
    {}

    small_vector(const self& right) noexcept
        : self(const_span_type(right))
    {}

    small_vector(const_span_type right_span) noexcept
        : self()
    {
        assign(right_span);
    }

    small_vector(self&& right) noexcept
        : self()
    {
        _move_assign(std::move(right));
    }

    self& operator = (const self& right) noexcept
    {
        if (this != &right)
        {
            assign(right);
        }

        return *this;
    }

    self& operator = (const_span_type right) noexcept
    {
        assign(right);
        return *this;
    }

    self& operator = (self&& right) noexcept
    {
        if (this != &right)
        {
            _move_assign(std::move(right));
        }

        return *this;
    }

    void assing(const_pointer source, size_type source_size) noexcept
    {
        clear();

        if (try_reserve(source_size))
        {
            std::uninitialized_copy_n(source, source_size, data_);
            size_ = source_size;
        }
    }

    void assign(const_span_type right) noexcept
    {
        assing(right.data(), right.size());
    }

    const_iterator insert(const_iterator position, value_type item) noexcept
    {
        D_ASSERT(position >= cbegin() && position <= cend());

        const auto position_index = (position - data_);

        iterator result{ nullptr };

        if (try_reserve(size() + 1u))
        {
            result = data_ + position_index;
            const auto last_position = data_ + size_;

            if (result == last_position)
            {
                new (result) value_type(std::move(item));
                ++size_;
            }
            else
            {
                const auto last_valid_position = std::prev(last_position);
                new (last_position) value_type(std::move(*last_valid_position));
                ++size_;

                std::move_backward(result, last_valid_position, last_position);
                *result = std::move(item);
            }
        }
        else
        {
            result = data_ + size_;
        }

        return result;
    }

    size_t erase(const_iterator first, const_iterator last) noexcept
    {
        D_ASSERT(last >= first);
        D_ASSERT(first >= cbegin());
        D_ASSERT(last <= cend());

        struct collector
        {
            pointer data;
            size_type size;

            ~collector() noexcept
            {
                std::destroy_n(data, size);
            }
        };

        collector temp{ data_, std::exchange(size_, 0u) };

        const auto erasable_data = std::move
        (
            temp.data + (last - temp.data),
            temp.data + temp.size,
            temp.data + (first - temp.data)
        );

        const auto new_size = narrow_cast<size_t>(erasable_data - temp.data);

        size_ = new_size;

        temp.data = erasable_data;
        temp.size -= new_size;
        return temp.size;
    }

    bool try_reserve(size_type new_capacity) noexcept
    {
        const auto capacity = self::capacity();
        const auto already_reserved = new_capacity <= capacity;
        return already_reserved || _growth_reallocate(capacity, new_capacity);
    }

    void reserve(size_t new_capacity) noexcept
    {
        try_reserve(new_capacity);
    }

    void shrink_to_fit() noexcept
    {
        if (data_ != small_ && size_ < big_.size())
        {
            if (size_ <= small_size)
            {
                struct collector
                {
                    uninitialized_dynarray_type data;
                    size_type size;

                    ~collector() noexcept
                    {
                        std::destroy_n(data.data(), size);
                    }
                };

                const collector temp{ std::move(big_), std::exchange(size_, 0u) };

                data_ = small_;
                std::uninitialized_move_n(temp.data.data(), temp.size, small_);
                size_ = temp.size;
            }
            else
            {
                _reallocate(size_);
            }
        }
    }

    void push_back(value_type item) noexcept
    {
        if (try_reserve(size() + 1u))
        {
            const auto last_position = data_ + size_;
            new (last_position) value_type(std::move(item));
            ++size_;
        }
    }

    void pop_back() noexcept
    {
        D_ASSERT(size_);
        const auto new_size = std::exchange(size_, 0u) - 1u;
        std::destroy_at(data_ + new_size);
        size_ = new_size;
    }

    void erase(const_iterator position) noexcept
    {
        erase(position, std::next(position));
    }

    void clear() noexcept
    {
        std::destroy_n(data_, std::exchange(size_, 0u));
    }

    constexpr size_t capacity() const noexcept
    {
        return (data_ == small_) ? small_size : big_.size();
    }

    constexpr size_type size() const noexcept
    {
        return size_;
    }

    constexpr pointer data() noexcept
    {
        return data_;
    }

    constexpr const_pointer data() const noexcept
    {
        return data_;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return data_;
    }

    constexpr const_iterator cend() const noexcept
    {
        return _end();
    }

    constexpr const_iterator begin() const noexcept
    {
        return cbegin();
    }

    constexpr const_iterator end() const noexcept
    {
        return cend();
    }

    constexpr iterator begin() noexcept
    {
        return data_;
    }

    constexpr iterator end() noexcept
    {
        return const_cast<pointer>(_end());
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return { cend() };
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return { cbegin() };
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return crbegin();
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
        return crend();
    }

    constexpr reverse_iterator rbegin() noexcept
    {
        return { end() };
    }

    constexpr reverse_iterator rend() noexcept
    {
        return { begin() };
    }

    constexpr const_reference cfront() const noexcept
    {
        return *data_;
    }

    constexpr const_reference cback() const noexcept
    {
        return *std::prev(_end());
    }

    constexpr const_reference front() const noexcept
    {
        return cfront();
    }

    constexpr const_reference back() const noexcept
    {
        return cback();
    }

    constexpr reference front() noexcept
    {
        return const_cast<reference>(cfront());
    }

    constexpr reference back() noexcept
    {
        return const_cast<reference>(cback());
    }

    constexpr const_reference operator[](size_type index) const noexcept
    {
        return value(index);
    }

    constexpr reference operator[](size_type index) noexcept
    {
        return value(index);
    }

    constexpr const_reference cvalue(size_type index) const noexcept
    {
        return data_[index];
    }

    constexpr const_reference value(size_type index) const noexcept
    {
        return cvalue(index);
    }

    constexpr reference value(size_type index) noexcept
    {
        return const_cast<reference>(cvalue(index));
    }

    constexpr bool is_small() const noexcept
    {
        return data_ == small_;
    }

    ~small_vector() noexcept
    {
        clear();

        if (data_ != small_)
        {
            big_.~uninitialized_dynarray();
        }
    }

private:
    void _move_assign(self&& right) noexcept
    {
        if (right.data_ == right.small_)
        {
            clear();

            const auto new_size = right.size_;
            std::uninitialized_move_n(right.data_, new_size, data_);
            size_ = new_size;
        }
        else
        {
            if (data_ == small_)
            {
                clear();
                new (&big_) uninitialized_dynarray_type(std::move(right.big_));
            }
            else
            {
                big_.swap(right.big_);
            }

            right.data_ = right.big_.data();
            data_ = big_.data();
            std::swap(size_, right.size_);
        }
    }

    bool _reallocate(size_type new_capacity) noexcept
    {
        D_ASSERT(new_capacity > small_size);

        const auto new_data = typed_memory_allocation<T>(new_capacity);

        if (new_data)
        {
            uninitialized_dynarray_type temp(attach_memory_construct, new_data, new_capacity);

            std::uninitialized_move_n(data_, size_, new_data);

            const auto size = std::exchange(size_, 0u);
            std::destroy_n(data_, size);

            if (data_ == small_)
            {
                new (&big_) uninitialized_dynarray_type(std::move(temp));
            }
            else
            {
                big_.swap(temp);
            }

            data_ = big_.data();
            size_ = size;
        }

        return !!new_data;
    }

    bool _growth_reallocate(size_type prev, size_type next) noexcept
    {
        D_ASSERT(prev < next);
        return _reallocate(std::max(next, optimal_geometric_growth(prev)));
    }

    constexpr const_pointer _end() const noexcept
    {
        return data_ + size_;
    }

private:
    union
    {
        value_type small_[small_size];
        uninitialized_dynarray_type big_;
    };
    pointer data_;
    size_t size_;
};
