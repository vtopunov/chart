#pragma once

#include <memory>

#include <core/buffer.h>
#include <core/span.h>

#undef min
#undef max


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast)

template<class It>
constexpr It back_move(It to, It back) noexcept
{
    constexpr auto is_trivially_copyable =
        std::is_trivially_copyable_v<typename std::iterator_traits<It>::value_type>;

    if constexpr (is_trivially_copyable)
    {
        auto temp = std::move(*back);
        std::copy(to, back, std::next(to));
        *to = std::move(temp);
    }
    else
    {
        while (back != to)
        {
            auto& temp = *back;
            swap(*--back, temp);
        }
    }

    return to;
}

template<class size_type> [[nodiscard]]
constexpr size_type optimal_memory_growth(size_type value) noexcept
{
    static_assert(std::is_unsigned_v<size_type>);
    constexpr size_type factor = 2;
    constexpr auto max_size = numeric_max_v<size_type>;
    constexpr auto overflow = max_size / factor;
    return (value < overflow) ? (factor * value) : max_size;
}

template<class size_type> [[nodiscard]]
constexpr size_type optimal_capacity_limit(size_type expected_capacity) noexcept
{
    return optimal_memory_growth(optimal_memory_growth(expected_capacity));
}

struct attach_construct_t
{};

constexpr attach_construct_t attach_construct{};

template<class T>
constexpr auto small_size_v = std::max(sizeof(buffer<T>) / sizeof(T), 1_uz);


template<class T, size_t N = small_size_v<T>, class Buffer = buffer<T>>
class small_vector
{
    using self = small_vector;

public:
    static_assert(N > 0_uz);

    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using span_type = span<value_type>;
    using const_span_type = span<std::add_const_t<value_type>>;
    using buffer_type = Buffer;
    using size_type = typename buffer_type::size_type;

    static constexpr size_type static_size{ N };

    constexpr small_vector() noexcept
        : data_{ static_ }
        , size_{ 0_uz }
    {}

    small_vector(const self& right) noexcept
        : self{ static_cast<const_span_type>(right) }
    {}

    small_vector(const_span_type right) noexcept
        : self{}
    {
        if (right.size() > static_size)
        {
            const auto ok = _try_reallocate(right.size());
            D_ASSERT(ok);
            if (D_UNLIKELY(!ok)) D_ATTRIB_UNLIKELY
            {
                return;
            }
        }

        _copy_initialization_elements(right);
    }

    constexpr small_vector(attach_construct_t, buffer_type mem) noexcept
        : self{}
    {
        _dynamic_buffer_construct(mem);
    }

    small_vector(attach_construct_t, self& right) noexcept
        : self{}
    {
        if (right.is_static())
        {
            size_ = right._uninitialized_move_to(static_);
        }
        else
        {
            _dynamic_construct(right);
        }
    }

    small_vector(self&& right) noexcept
        : self{attach_construct, right}
    {}

    self& operator = (const self& right) noexcept
    {
        if (D_LIKELY(this != std::addressof(right))) D_ATTRIB_LIKELY
        {
            D_ASSERT_OR_UNUSED(try_assign(right));
        }

        return *this;
    }

    self& operator = (const_span_type right) noexcept
    {
        D_ASSERT_OR_UNUSED(try_assign(right));
        return *this;
    }

    self& operator = (self&& right) noexcept
    {
        if (D_LIKELY(this != std::addressof(right))) D_ATTRIB_LIKELY
        {
            if (right.is_static())
            {
                _destroy();
                size_ = right._uninitialized_move_to(static_);
            }
            else
            {
                if (is_static())
                {
                    D_UNUSED(_destroy_elements());
                    _dynamic_construct(right);
                }
                else
                {
                    _dynamic_swap(right);
                }
            }
        }

        return *this;
    }

    [[nodiscard]]
    bool try_assign(const_span_type right) noexcept
    {
        D_UNUSED(_destroy_elements());

        if (right.size() > capacity())
        {
            if (D_UNLIKELY(!_try_reallocate(right.size()))) D_ATTRIB_UNLIKELY
            {
                return false;
            }
        }
        else
        {
            _collect(right.size());
        }

        _copy_initialization_elements(right);

        return true;
    }

    void attach_buffer(buffer_type mem) noexcept
    {
        D_ASSERT(mem.size() > capacity());
        _attach_buffer(mem);
    }

    [[nodiscard]]
    buffer_type release_buffer() noexcept
    {
        D_UNUSED(_destroy_elements());

        if (is_dynamic())
        {
            return _release_buffer();
        }

        return {};
    }

    template<class... Args>
    [[nodiscard]] const_iterator try_emplace(const_iterator position, Args&&... args) noexcept
    {
        D_ASSERT(position >= cbegin());
        D_ASSERT(position <= cend());

        const auto position_index = (position - data_);

        if (D_LIKELY(const auto last = try_emplace_back(std::forward<Args>(args)...))) D_ATTRIB_LIKELY
        {
            return back_move(data_ + position_index, last);
        }

        return nullptr;
    }

    size_type erase(const_iterator first, const_iterator last) noexcept
    {
        class collector
        {
        public:
            constexpr explicit collector(small_vector& store) noexcept
                : store_{ store }
                , locked_data_{ store.data_ }
                , locked_size_{ store._release_size() }
            {}

            D_DISABLE_COPY_MOVE(collector);

            [[nodiscard]]
            constexpr size_type erase(const_pointer first, const_pointer last) noexcept
            {
                return _set_removed_data(_remove_elements(first, last));
            }

            ~collector() noexcept
            {
                std::destroy_n(locked_data_, locked_size_);
                store_._collect();
            }

        private:
            [[nodiscard]]
            constexpr size_type _set_removed_data(pointer removed_data) noexcept
            {
                const auto new_size = narrow_cast<size_t>(removed_data - locked_data_);
                locked_data_ = removed_data;
                
                const auto count_of_erased = locked_size_ - new_size;
                locked_size_ = count_of_erased;

                store_.size_ = new_size;

                return count_of_erased;
            }

            [[nodiscard]]
            constexpr pointer _remove_elements(const_pointer first, const_pointer last) const noexcept
            {
                D_ASSERT(last >= first);
                D_ASSERT(first >= locked_data_);
                
                const auto last_last = locked_data_ + locked_size_;
                D_ASSERT(last <= last_last);

                return std::move
                (
                    const_cast<pointer>(last), 
                    last_last, 
                    const_cast<pointer>(first)
                );
            }

        private:
            small_vector& store_;
            pointer locked_data_;
            size_type locked_size_;
        };

        return collector{ *this }.erase(first, last);
    }

    [[nodiscard]]
    bool try_reserve(size_type new_capacity) noexcept
    {
        return (new_capacity <= capacity()) || _try_reallocate(new_capacity);
    }

    void reserve(size_type new_capacity) noexcept
    {
        D_ASSERT_OR_UNUSED(try_reserve(new_capacity));
    }

    [[nodiscard]]
    bool try_shrink_to_fit() noexcept
    {
        bool ok{ true };

        if (is_dynamic() && dynamic_.size() > size_)
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
    [[nodiscard]] pointer try_emplace_back(Args&&... args) noexcept
    {
        if (D_LIKELY(_try_indeterminate_reserve(size() + 1_uz))) D_ATTRIB_LIKELY
        {
            const auto last = data_ + size_;
            new (last) value_type{ std::forward<Args>(args)... };
            ++size_;
            return last;
        }

        return nullptr;
    }

    template<class... Args>
    reference emplace_back(Args&&... args) noexcept
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

    void clear() noexcept
    {
        D_UNUSED(_destroy_elements());
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
        return *(cend()-1_uz);
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

    ~small_vector() noexcept
    {
        _destroy();
    }

private:
    [[nodiscard]]
    constexpr size_type _release_size() noexcept
    {
        return std::exchange(size_, 0_uz);
    }

    constexpr void _set_dynamic() noexcept
    {
        D_ASSERT(dynamic_.size() > static_size);
        data_ = dynamic_.data();
    }

    constexpr void _destroy_dynamic() noexcept
    {
        D_ASSERT(is_dynamic());
        data_ = static_;
        std::destroy_at(std::addressof(dynamic_));
    }

    [[nodiscard]]
    buffer_type _release_buffer() noexcept
    {
        D_ASSERT(is_dynamic());
        buffer_type temp{ std::move(dynamic_) };
        _destroy_dynamic();
        return temp;
    }

    [[nodiscard]]
    size_type _destroy_elements() noexcept
    {
        const auto size = _release_size();
        std::destroy_n(data_, size);
        return size;
    }

    void _destroy() noexcept
    {
        D_UNUSED(_destroy_elements());

        if (is_dynamic())
        {
            _destroy_dynamic();
        }
    }

    void _copy_initialization_elements(const_span_type source) noexcept
    {
        D_ASSERT(0_uz == size_);
        D_ASSERT(capacity() >= source.size());
        std::uninitialized_copy_n(source.data(), source.size(), data_);
        size_ = source.size();
    }

    [[nodiscard]]
    size_type _uninitialized_move_to(span_type destination) noexcept
    {
        D_ASSERT(size_ <= destination.size());
        std::uninitialized_move_n(data_, size_, destination.data());
        return _destroy_elements();
    }

    constexpr void _dynamic_buffer_construct(buffer_type& dynamic) noexcept
    {
        D_ASSERT(is_static());
        new (std::addressof(dynamic_)) buffer_type(std::move(dynamic));
        _set_dynamic();
    }

    constexpr void _dynamic_buffer_swap(buffer_type& dynamic) noexcept
    {
        D_ASSERT(is_dynamic());
        dynamic_.swap(dynamic);
        _set_dynamic();
    }

    constexpr void _dynamic_construct(self& right) noexcept
    {
        D_ASSERT(0_uz == size_);
        D_ASSERT(right.is_dynamic());
        _dynamic_buffer_construct(right.dynamic_);
        right._destroy_dynamic();
        size_ = right._release_size();
    }

    constexpr void _dynamic_swap(self& right) noexcept
    {
        D_ASSERT(right.is_dynamic());
        _dynamic_buffer_swap(right.dynamic_);
        right._set_dynamic();
        std::swap(size_, right.size_);
    }

    void _attach_buffer(buffer_type& mem) noexcept
    {
        size_ = _uninitialized_move_to(mem);

        if (is_static())
        {
            _dynamic_buffer_construct(mem);
        }
        else
        {
            _dynamic_buffer_swap(mem);
        }
    }

    [[nodiscard]]
    bool _try_reallocate(size_type new_capacity) noexcept
    {
        if (buffer_type temp{buffer_construct, new_capacity}; D_LIKELY(temp)) D_ATTRIB_LIKELY
        {
            _attach_buffer(temp);
            return true;
        }

        return false;
    }

    void _switch_to_static() noexcept
    {
        class collector
        {
        public:
            explicit collector(small_vector& store) noexcept
                : store_{ store }
                , data_{ store_._release_buffer() }
                , size_{ store_._release_size() }
            {}

            D_DISABLE_COPY_MOVE(collector);

            void uninitialized_move_to_static() noexcept
            {
                D_ASSERT(size_ <= static_size);
                static_assert(std::is_same_v<decltype(data_.data()), pointer>);
                std::uninitialized_move_n(data_.data(), size_, store_.data_);
                store_.size_ = size_;
            }

            ~collector() noexcept
            {
                std::destroy_n(data_.data(), size_);
            }

        private:
            small_vector& store_;
            buffer_type data_;
            size_type size_;
        };

        collector temp{ *this };
        temp.uninitialized_move_to_static();
    }

    [[nodiscard]]
    bool _try_indeterminate_reserve(size_type require_capacity) noexcept
    {
        const auto old_capacity = capacity();
        return require_capacity <= old_capacity
            || _try_reallocate(std::max(require_capacity, optimal_memory_growth(old_capacity)));
    }

    [[nodiscard]]
    bool _try_collect(size_type expected_capacity) noexcept
    {
        bool ok{ true };

        if (is_dynamic())
        {
            constexpr auto min_capacity_limit = optimal_capacity_limit(static_size);

            const auto is_expected_small = (expected_capacity <= static_size);

            const auto capacity_limit
                = is_expected_small
                ? min_capacity_limit
                : optimal_capacity_limit(expected_capacity);

            const auto current_capacity = dynamic_.size();

            if (current_capacity > capacity_limit)
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

    void _collect(size_type expected_capacity) noexcept
    {
        D_ASSERT_OR_UNUSED(_try_collect(expected_capacity));
    }

    void _collect() noexcept
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

D_WARNING_POP