#pragma once

#include <memory>

#include <core/buffer.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast)
D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized)

namespace private_detail_back_move
{
    template<class It>
    constexpr void back_move(It to, It back) noexcept
    {
        using Value = typename std::iterator_traits<It>::value_type;

        constexpr auto is_trivially_assignable
            = std::is_trivially_assignable_v<std::add_lvalue_reference_t<Value>, Value>;

        if constexpr (is_trivially_assignable)
        {
            std::move_backward(to, back, std::next(back));
        }
        else
        {
            while (back != to)
            {
                auto& temp = *back;
                temp = std::move(*--back);
            }
        }
    }
}



[[nodiscard]] constexpr size_t optimal_memory_growth(size_t value) noexcept
{
    D_ASSERT(0_uz < value);
    return size_mul_or_max<3u>(value);
}

[[nodiscard]] constexpr size_t optimal_capacity_limit(size_t expected_capacity) noexcept
{
    D_ASSERT(0_uz < expected_capacity);
    return size_mul_or_max<9u>(expected_capacity);
}

template<class T>
constexpr auto small_vector_default_static_size_v = (std::max)(sizeof(buffer<T>) / sizeof(T), 1_uz);

template
<
    class T,
    size_t N = small_vector_default_static_size_v<T>,
    class Buffer = buffer<T>
>
class small_vector
{
    using self = small_vector;

public:
    using value_type = T;
    using const_value_type = const value_type;
    using pointer = value_type*;
    using const_pointer = const_value_type*;
    using reference = value_type&;
    using const_reference = const_value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using span_type = span<value_type>;
    using const_span_type = span<const_value_type>;
    using buffer_type = Buffer;
    using size_type = size_t;
    using view_type = const_span_type;
    using null_type = nullmem_t;

    static constexpr size_type static_size{ N };
    static constexpr auto min_capacity_limit = optimal_capacity_limit(static_size);

    static_assert(0_uz < static_size);
    static_assert(std::is_same_v<size_t, typename buffer_type::size_type>);
    static_assert(!std::is_copy_constructible_v<buffer_type>);
    static_assert(!std::is_copy_assignable_v<buffer_type>);

    constexpr small_vector() noexcept
        : data_{ static_ }
        , size_{ 0_uz }
    {}

    constexpr small_vector(null_type) noexcept
        : self{}
    {}

    constexpr small_vector(const self& right) noexcept
        : self{ static_cast<const_span_type>(right) }
    {}

    constexpr small_vector(const_span_type right) noexcept
        : self{}
    {
        if (right.size() > static_size)
        {
            buffer_type temp{ right.size() };
            D_ASSERT(temp);
            if (!temp) [[unlikely]]
            {
                return;
            }

            _dynamic_buffer_construct(std::move(temp));
        }

        _copy_initialization(right);
    }

    constexpr explicit small_vector(buffer_type&& mem) noexcept
        : self{}
    {
        _dynamic_buffer_construct(std::move(mem));
    }

    constexpr small_vector(self&& right) noexcept
        : self{}
    {
        if (right.is_static())
        {
            std::uninitialized_move_n(right.data_, right.size_, static_);
            size_ = right.size_;
        }
        else
        {
            _dynamic_construct(std::move(right));
        }
    }

    constexpr self& operator = (null_type) noexcept
    {
        clear();
        return *this;
    }

    constexpr self& operator = (const self& right) noexcept
    {
        if (this != std::addressof(right)) [[likely]]
        {
            D_ASSERT_OR_UNUSED(try_assign(right));
        }

        return *this;
    }

    constexpr self& operator = (self&& right) noexcept
    {
        if (this != std::addressof(right)) [[likely]]
        {
            std::destroy_n(data_, size_);

            if (right.is_static())
            {
                size_ = {};
                
                if(is_dynamic())
                {
                    if(dynamic_.size() > min_capacity_limit) [[unlikely]]
                    {
                        data_ = static_;
                        std::destroy_at(std::addressof(dynamic_));
                    }
                }

                std::uninitialized_move_n(right.data_, right.size_, data_);
                size_ = right.size_;
            }
            else
            {
                if (is_static())
                {
                    _dynamic_construct(std::move(right));
                }
                else
                {
                    _dynamic_buffer_assign(std::move(right.dynamic_));
                    size_ = std::exchange(right.size_, {});
                    right.data_ = right.dynamic_.data();
                }
            }
        }

        return *this;
    }

    constexpr explicit operator bool() const noexcept
    {
        return !!size_;
    }

    [[nodiscard]]
    bool try_assign(const_span_type right) noexcept
    {
        std::destroy_n(data_, std::exchange(size_, {}));

        if (right.size() > capacity())
        {
            buffer_type temp{ right.size() };
            if (!temp) [[unlikely]]
            {
                return false;
            }

            _set_buffer(std::move(temp));
        }
        else
        {
            _collect(right.size());
        }

        _copy_initialization(right);

        return true;
    }

    constexpr pointer attach_buffer(buffer_type&& mem) noexcept
    {
        D_ASSERT(mem.size() >= size());

        const auto first = data_;
        const auto last = first + size_;
        const auto result = std::uninitialized_move(first, last, mem.data());
        std::destroy(first, last);
        _set_buffer(std::move(mem));
        return result;
    }

    class temp_vector
    {
    public:
        constexpr temp_vector(buffer_type&& data) noexcept
            : data_{ std::move(data) }
            , end_of_array_{ data_.data() }
        {}

        constexpr temp_vector(buffer_type&& data, size_type size) noexcept
            : data_{ std::move(data) }
            , end_of_array_{ data_.data() + size }
        {
            D_ASSERT(size <= data_.size());
        }

        constexpr temp_vector(size_t max_size) noexcept
            : data_{ max_size }
            , end_of_array_{ data_.data() }
        {}

        D_DISABLE_COPYMOVE_CA(temp_vector);

        constexpr ~temp_vector() noexcept
        {
            D_ASSERT(end_of_array_ >= data_.data());
            D_ASSERT(narrow<size_type>(end_of_array_ - data_.data()) <= data_.size());
            std::destroy(data_.data(), end_of_array_);
        }

        constexpr void emplace_push_range(pointer first, pointer last) noexcept
        {
            end_of_array_ = std::uninitialized_move(first, last, end_of_array_);
        }

        template<class... Args>
        constexpr pointer emplace_push(Args&&... args) noexcept
        {
            const auto result = end_of_array_;
            new (result) value_type{ std::forward<Args>(args)... };
            ++end_of_array_;
            return result;
        }

        constexpr void uninitialized_move_to(pointer to) noexcept
        {
            std::uninitialized_move(data_.data(), end_of_array_, to);
        }

        [[nodiscard]] constexpr buffer_type& buffer_ref() noexcept
        {
            return data_;
        }

        constexpr void leave() noexcept
        {
            end_of_array_ = data_.data();
        }

        constexpr explicit operator bool() const noexcept
        {
            return !!data_;
        }

    private:
        buffer_type data_;
        pointer end_of_array_;
    };

    [[nodiscard]]
    constexpr buffer_type release_buffer() noexcept
    {
        if (static_ != data_)
        {
            {
                const auto data = std::exchange(data_, static_);
                const auto size = std::exchange(size_, {});
                std::destroy_n(data, size);
            }

            buffer_type temp{ std::move(dynamic_) };
            std::destroy_at(std::addressof(dynamic_));
            return temp;
        }

        return {};
    }

    template<class... Args>
    [[nodiscard]] constexpr const_iterator try_emplace(const_iterator position, Args&&... args) noexcept
    {
        D_ASSERT(position >= cbegin());
        D_ASSERT(position <= cend());

        const auto mut_postion = const_cast<pointer>(position);
        const auto size = size_;
        const auto new_size = size + 1_uz;
        const auto old_capacity = capacity();

        if (size < old_capacity) [[likely]]
        {
            using private_detail_back_move::back_move;

            const auto last = data_ + size;

            if constexpr (std::conjunction_v<
                std::is_trivially_default_constructible<value_type>,
                std::is_trivially_destructible<value_type>
            >)
            {
                back_move(mut_postion, last);
            }
            else
            {
                if (mut_postion != last)
                {
                    {
                        const auto p_back = std::prev(last);
                        new (last) value_type(std::move(*p_back));
                        back_move(mut_postion, p_back);
                    }
                    std::destroy_at(mut_postion);
                }
            }

            new (mut_postion) value_type{ std::forward<Args>(args)... };
            size_ = new_size;
            return position;
        }

        if (temp_vector temp{ optimal_memory_growth(old_capacity) }) [[likely]]
        {
            const auto first = data_;
            const auto last = first + size;

            temp.emplace_push_range(first, mut_postion);
            const auto result = temp.emplace_push(std::forward<Args>(args)...);
            temp.emplace_push_range(mut_postion, last);
            std::destroy(first, last);
            _set_buffer(std::move(temp.buffer_ref()));
            temp.leave();

            size_ = new_size;
            return result;
        }

        return nullptr;
    }

    constexpr size_type erase(const_iterator first, const_iterator last) noexcept
    {
        D_ASSERT(last >= first);
        D_ASSERT(first >= data_);
        D_ASSERT(last <= cend());

        const auto removed_data = std::move(const_cast<pointer>(last), data_ + size_, const_cast<pointer>(first));
        const auto new_size = narrow<size_type>(removed_data - data_);
        const auto count_of_erased = size_ - new_size;
        size_ = new_size;
        std::destroy_n(removed_data, count_of_erased);
        _collect(new_size);

        return count_of_erased;
    }

    [[nodiscard]]
    constexpr bool try_reserve(size_type new_capacity) noexcept
    {
        return (new_capacity <= capacity()) || _try_reallocate(new_capacity);
    }

    void reserve(size_type new_capacity) noexcept
    {
        D_ASSERT_OR_UNUSED(try_reserve(new_capacity));
    }

    [[nodiscard]]
    constexpr bool try_shrink_to_fit() noexcept
    {
        bool ok{ true };

        if (is_dynamic() && (dynamic_.size() > size_))
        {
            if (size_ <= static_size)
            {
                _switch_to_static();
            }
            else
            {
                ok = _try_reallocate(size_);
            }
        }

        return ok;
    }

    void shrink_to_fit() noexcept
    {
        D_ASSERT_OR_UNUSED(try_shrink_to_fit());
    }

    template<class... Args>
    [[nodiscard]] constexpr pointer try_emplace_back(Args&&... args) noexcept
    {
        const auto size = size_;
        const auto new_size = size + 1u;
        const auto old_capacity = capacity();

        if (size < old_capacity) [[likely]]
        {
            const auto last = data_ + size;
            new (last) value_type{ std::forward<Args>(args)... };
            size_ = new_size;
            return last;
        }

        if (buffer_type temp{ optimal_memory_growth(old_capacity) }) [[likely]]
        {
            const auto new_last = attach_buffer(std::move(temp));
            new (new_last) value_type{ std::forward<Args>(args)... };
            size_ = new_size;
            return new_last;
        }

        return nullptr;
    }

    template<class... Args>
    constexpr reference emplace_back(Args&&... args) noexcept
    {
        const auto last = try_emplace_back(std::forward<Args>(args)...);
        D_ASSERT(last);
        return *last;
    }

    void pop_back() noexcept
    {
        D_ASSERT(size_);
        std::destroy_at(--size_ + data_);
        _collect();
    }

    void erase(const_iterator position) noexcept
    {
        erase(position, std::next(position));
    }

    constexpr void clear() noexcept
    {
        std::destroy_n(data_, std::exchange(size_, {}));
        _collect(0_uz);
    }

    [[nodiscard]]
    constexpr size_type capacity() const noexcept
    {
        return is_static() ? static_size : dynamic_.size();
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]]
    constexpr pointer data() noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return data_ + size_;
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
        return const_cast<iterator>(cbegin());
    }

    [[nodiscard]]
    constexpr iterator end() noexcept
    {
        return const_cast<iterator>(cend());
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        return *data_;
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
        return data_[index];
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

    [[nodiscard]]
    constexpr bool is_static() const noexcept
    {
        return static_ == data_;
    }

    [[nodiscard]]
    constexpr bool is_dynamic() const noexcept
    {
        return !is_static();
    }

    constexpr ~small_vector() noexcept
    {
        std::destroy_n(data_, size_);

        if (is_dynamic())
        {
            std::destroy_at(std::addressof(dynamic_));
        }
    }

private:
    void _copy_initialization(const_span_type source) noexcept
    {
        D_ASSERT(0_uz == size_);
        D_ASSERT(capacity() >= source.size());
        std::uninitialized_copy_n(source.data(), source.size(), data_);
        size_ = source.size();
    }

    constexpr void _dynamic_buffer_construct(buffer_type&& dynamic) noexcept
    {
        D_ASSERT(is_static());
        D_ASSERT(dynamic && dynamic.size() >= static_size);
        new (std::addressof(dynamic_)) buffer_type(std::move(dynamic));
        data_ = dynamic_.data();
    }

    constexpr void _dynamic_buffer_assign(buffer_type&& dynamic) noexcept
    {
        D_ASSERT(is_dynamic());
        D_ASSERT(dynamic && dynamic.size() >= static_size);
        dynamic_ = std::move(dynamic);
        data_ = dynamic_.data();
    }

    constexpr void _dynamic_construct(self&& right) noexcept
    {
        _dynamic_buffer_construct(std::move(right.dynamic_));
        size_ = std::exchange(right.size_, {});
        right.data_ = right.static_;
        std::destroy_at(std::addressof(right.dynamic_));
    }

    constexpr void _set_buffer(buffer_type&& mem) noexcept
    {
        if (is_static())
        {
            _dynamic_buffer_construct(std::move(mem));
        }
        else
        {
            _dynamic_buffer_assign(std::move(mem));
        }
    }

    [[nodiscard]]
    constexpr bool _try_reallocate(size_type new_capacity) noexcept
    {
        if (buffer_type temp{ new_capacity }) [[likely]]
        {
            attach_buffer(std::move(temp));
            return true;
        }

        return false;
    }

    void _switch_to_static() noexcept
    {
        D_ASSERT(is_dynamic());
        D_ASSERT(size() <= static_size);

        data_ = static_;
        const auto size = std::exchange(size_, {});
        temp_vector temp{ std::move(dynamic_), size };
        std::destroy_at(std::addressof(dynamic_));
        temp.uninitialized_move_to(static_);
        size_ = size;
    }

    [[nodiscard]]
    constexpr bool _try_collect(size_type expected_capacity) noexcept
    {
        bool ok{ true };

        if (is_dynamic())
        {
            const auto is_expected_small = (expected_capacity <= static_size);

            const auto capacity_limit
                = is_expected_small
                ? min_capacity_limit
                : optimal_capacity_limit(expected_capacity);

            const auto current_capacity = dynamic_.size();

            if (current_capacity > capacity_limit) [[unlikely]]
            {
                if (is_expected_small)
                {
                    _switch_to_static();
                }
                else
                {
                    ok = _try_reallocate(expected_capacity);
                }
            }
        }

        return ok;
    }

    constexpr void _collect(size_type expected_capacity) noexcept
    {
        D_ASSERT_OR_UNUSED(_try_collect(expected_capacity));
    }

    constexpr void _collect() noexcept
    {
        _collect(size_);
    }

private:
    union
    {
        value_type static_[static_size];
        buffer_type dynamic_;
    };
    pointer data_;
    size_type size_;
};

static_assert(sizeof(small_vector<char>) == small_size_v);

D_WARNING_POP