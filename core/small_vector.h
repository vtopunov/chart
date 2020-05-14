#pragma once

#include <vector>
#include <algorithm>
#include <memory>

#include <core/defs.h>
#include <core/span.h>

template<class T, size_t N>
class small_vector
{
public:
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
    using dynarray_type = std::vector<value_type>;
    using const_iterator_dynarray = typename dynarray_type::const_iterator;
    using span_type = span<value_type>;

    constexpr small_vector() noexcept
        : data_{ std::data(static_), 0u }
    {}

    small_vector(const small_vector& v) noexcept
    {
        const auto copy_it = v.cbegin();
        const auto copy_size = v.size();

        if ( copy_size <= small_size )
        {
            std::uninitialized_copy_n(copy_it, copy_size, std::data(static_));
            new ( std::addressof(data_) ) span_type{ std::data(static_), copy_size };
        }
        else
        {
            new ( std::addressof(dynamic_) ) dynarray_type{ v.dynamic_ };
            new ( std::addressof(data_) ) span_type{ dynamic_ };
        }
    }

    small_vector(small_vector&& v) noexcept
    {
        const auto move_it = v.cbegin();
        const auto move_size = v.size();

        if ( move_size <= small_size )
        {
            std::uninitialized_move_n(move_it, move_size, std::data(static_));
            new ( std::addressof(data_) ) span_type{ std::data(static_), move_size };
        }
        else
        {
            new ( std::addressof(dynamic_) ) dynarray_type{ std::move(v.dynamic_) };
            new ( std::addressof(data_) ) span_type{ dynamic_ };
        }
    }

    small_vector& operator = (const small_vector& v) noexcept
    {
        if ( this != std::addressof(v) )
        {
            clear();
            
            if ( is_small() )
            {
                const auto copy_it = v.cbegin();
                const auto copy_size = v.size();

                if ( copy_size <= small_size )
                {
                    data_ = { std::data(static_), copy_size };
                    std::uninitialized_copy_n(copy_it, copy_size, std::data(static_));
                    return *this;
                }

                _switch_to_dynamic();
            }

            if ( v.is_small() )
            {
                dynamic_.assign(v.begin(), v.end());
            }
            else
            {
                dynamic_ = std::move(v.dynamic_);
            }

            data_ = dynamic_;
        }

        return *this;
    }

    small_vector& operator = (small_vector&& v) noexcept
    {
        if ( this != std::addressof(v) )
        {
            if ( is_small() )
            {
                const auto move_it = v.begin();
                const auto move_size = v.size();

                if ( move_size <= small_size )
                {
                    const auto data = std::data(static_);
                    const auto size = data_.size();
                    data_ = { data, move_size };

                    if ( move_size > size )
                    {
                        const auto slice_it = move_it + size;

                        std::uninitialized_move
                        (
                            slice_it, move_it + move_size,
                            std::move(move_it, slice_it, data)
                        );
                    }
                    else
                    {
                        std::destroy
                        (
                            std::move(move_it, move_it + move_size, data),
                            data + size
                        );
                    }

                    return *this;
                }

                _switch_to_dynamic();
            }

            if ( v.is_small() )
            {
                dynamic_.assign(std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
            }
            else
            {
                dynamic_ = std::move(v.dynamic_);
            }

            data_ = dynamic_;
        }

        return *this;
    }

    const_iterator insert(const_iterator position, value_type item) noexcept
    {
        D_ASSERT(position >= cbegin() && position <= cend());

        const auto index = position - cbegin();

        if ( is_small() )
        {
            const auto size = data_.size();

            if ( size < small_size )
            {
                {
                    const auto mutable_postion = std::begin(static_) + index;
                    const auto last_position = std::begin(static_) + size;

                    std::move_backward
                    (
                        mutable_postion, last_position,
                        std::uninitialized_default_construct_n(last_position, 1)
                    );

                    *mutable_postion = std::move(item);
                }

                data_ = { std::begin(static_), size + 1u };

                return position;
            }

            _switch_to_dynamic();
        }

        dynamic_.insert(dynamic_.cbegin() + index, std::move(item));
        data_ = dynamic_;

        return data_.cbegin() + index;
    }

    size_t erase(const_iterator first, const_iterator last) noexcept
    {
        D_ASSERT(last >= first);
        D_ASSERT(first >= cbegin());
        D_ASSERT(last <= cend());

        const auto data = data_.data();
        const auto first_index = first - data;
        const auto last_index = last - data;

        const auto size = data_.size();
        const auto count_of_erased = narrow_cast<size_t>( last - first );
        const auto new_size = size - count_of_erased;
        
        if ( is_small() )
        {
            data_ = { std::data(static_), new_size };

            const auto new_end = std::move
            (
                data + last_index,
                data + size,
                data + first_index
            );

            std::destroy_n(new_end, count_of_erased);
        }
        else
        {
            data_ = { std::data(dynamic_), new_size };

            const auto dynamic_it = dynamic_.cbegin();
            dynamic_.erase
            (
                dynamic_it + first_index,
                dynamic_it + last_index
            );
        }

        return count_of_erased;
    }

    void reserve(size_t new_size) noexcept
    {
        if ( new_size > small_size )
        {
            if ( is_small() )
            {
                _switch_to_dynamic();
            }

            dynamic_.reserve(new_size);
            data_ = dynamic_;
        }
    }

    void shrink_to_fit() noexcept
    {
        if ( !is_small() )
        {
            if ( size() <= small_size )
            {
                _switch_to_static();
            }
            else
            {
                dynamic_.shrink_to_fit();
                data_ = dynamic_;
            }
        }
    }

    void push_back(value_type item) noexcept
    {
        insert(cend(), std::move(item));
    }

    void pop_back() noexcept
    {
        const auto last = cend();
        D_ASSERT(last > cbegin());
        erase(std::prev(last), last);
    }

    void erase(const_iterator position) noexcept
    {
        erase(position, std::next(position));
    }

    void clear() noexcept
    {
        if ( is_small() )
        {
            const auto data = data_;
            data_ = { std::data(static_), 0u };
            std::destroy(data.begin(), data.end());
        }
        else
        {
            data_ = { std::data(dynamic_), 0u };
            dynamic_.clear();
        }
    }

    constexpr size_t size() const noexcept
    {
        return data_.size();
    }

    constexpr size_t capacity() const noexcept
    {
        return ( is_small() ) ? small_size : dynamic_.capacity();
    }

    constexpr pointer data() noexcept
    {
        return data_.data();
    }

    constexpr const_pointer data() const noexcept
    {
        return data_.cdata();
    }

    constexpr iterator begin() noexcept
    {
        return data_.begin();
    }

    constexpr iterator end() noexcept
    {
        return data_.end();
    }

    constexpr const_iterator begin() const noexcept
    {
        return data_.cbegin();
    }

    constexpr const_iterator end() const noexcept
    {
        return data_.cend();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

    constexpr reverse_iterator rbegin() noexcept
    {
        return { end() };
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return { cend() };
    }

    constexpr reverse_iterator rend() noexcept
    {
        return { begin() };
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
        return { cbegin() };
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    constexpr reference front() noexcept
    {
        return data_.front();
    }

    constexpr const_reference front() const noexcept
    {
        return front();
    }

    constexpr const_reference back() noexcept
    {
        return data_.back();
    }

    constexpr const_reference back() const noexcept
    {
        return back();
    }

    constexpr reference operator[](size_t index) noexcept
    {
        return data_[index];
    }

    constexpr const_reference operator[](size_t index) const noexcept
    {
        return data_[index];
    }

    constexpr size_t max_size() const noexcept
    {
        return data_.max_size();
    }

    constexpr bool is_small() const noexcept
    {
        return cbegin() == std::cbegin(static_);
    }

    ~small_vector() noexcept
    {
        if ( is_small() )
        {
            _destroy_static();
        }
        else
        {
            _destroy_dynamic();
        }
    }

private:
    void _destroy_static() noexcept
    {
        const auto data = data_;
        data_ = { std::data(static_), 0u };
        std::destroy(data.begin(), data.end());
    }

    void _destroy_dynamic() noexcept
    {
        data_ = { std::data(static_), 0u };
        std::destroy_at(std::addressof(dynamic_));
    }

    void _switch_to_static() noexcept
    {
        D_ASSERT(!is_small() && dynamic_.size() <= small_size);
        dynarray_type dynamic{ std::move(dynamic_) };
        _destroy_dynamic();
        data_ = { std::data(static_), dynamic.size() };
        std::uninitialized_move(dynamic.begin(), dynamic.end(), static_);
    }

    void _switch_to_dynamic() noexcept
    {
        D_ASSERT(is_small());
        dynarray_type dynamic;
        dynamic.reserve(3u * small_size);
        dynamic.assign(std::make_move_iterator(begin()), std::make_move_iterator(end()));
        _destroy_static();
        new ( std::addressof(dynamic_) ) dynarray_type{ std::move(dynamic) };
        data_ = dynamic_;
    }

private:
    union
    {
        value_type static_[small_size];
        dynarray_type dynamic_;
    };
    span_type data_;
};
