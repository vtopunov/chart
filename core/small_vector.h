#pragma once

#include <core/memory.h>
#include <core/buffer.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast)
D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized)

namespace private_detail_small_vector
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

    template<class T, class Buffer>
    class temp_vector
    {
    public:
        using value_type = T;
        using buffer_type = Buffer;
        using pointer = value_type*;
        using size_type = typename buffer_type::size_type;

        constexpr temp_vector(buffer_type&& buffer, size_type size) noexcept
            : buffer_{ std::move(buffer) }
            , end_of_array_{ std::data(buffer_) + size }
        {
            D_ASSERT(size <= std::size(buffer_));
        }

        constexpr explicit temp_vector(size_type max_size) noexcept
            : buffer_{ max_size }
            , end_of_array_{ std::data(buffer_) }
        {}

        D_DISABLE_COPYMOVE_CA(temp_vector);

        constexpr ~temp_vector() noexcept
        {
            std::destroy(begin(), end());
        }

        constexpr void emplace_push_range(pointer first, pointer last) noexcept
        {
            D_ASSERT(u_distance(first, last) <= free_capacity());
            end_of_array_ = movenew(first, last, end_of_array_);
        }

        template<class... Args>
        constexpr pointer emplace_push(Args&&... args) noexcept
        {
            D_ASSERT(0u < free_capacity());
            const auto result = end_of_array_;
            ::construct_at(result, std::forward<Args>(args)...);
            ++end_of_array_;
            return result;
        }

        [[nodiscard]] constexpr size_t capacity() const noexcept
        {
            return std::size(buffer_);
        }

        [[nodiscard]] constexpr size_t free_capacity() const noexcept
        {
            return u_distance(size(), capacity());
        }

        [[nodiscard]] constexpr size_t size() const noexcept
        {
            return u_distance(begin(), end());
        }

        [[nodiscard]] constexpr pointer begin() const noexcept
        {
            return std::data(buffer_);
        }

        [[nodiscard]] constexpr pointer end() const noexcept
        {
            return end_of_array_;
        }

        [[nodiscard]] constexpr buffer_type& buffer_ref() noexcept
        {
            return buffer_;
        }

        constexpr void leave() noexcept
        {
            end_of_array_ = begin();
        }

        constexpr explicit operator bool() const noexcept
        {
            return !!buffer_;
        }

    private:
        buffer_type buffer_;
        pointer end_of_array_;
    };
}

namespace small_vector_exceptions
{
    constexpr const char bad_alloc_msg[]{ "badalloc" };
    constexpr size_t bad_alloc_msg_len{ sizeof(bad_alloc_msg) - 1u };
    static_assert(bad_alloc_msg_len <= 8u);

    constexpr struct
    {
        void operator () () const noexcept
        {
            D_ASSERT(ENOMEM);
            errno = ENOMEM;
        }
    } accept_bad_alloc;

    constexpr struct
    {
        template<class T>
        size_t operator () (T* data, size_t new_size) const noexcept
        {
            constexpr span<const char, bad_alloc_msg_len> bad_alloc_msg_span{ bad_alloc_msg };
            const auto msg = bad_alloc_msg_span.first(std::min(bad_alloc_msg_span.size(), new_size));
            copynew(msg, data);
            return msg.size();
        }
    } accept_and_write_bad_alloc;

    constexpr size_t msg_max_len{ bad_alloc_msg_len };
}

[[nodiscard]] constexpr size_t optimal_memory_growth(size_t value) noexcept
{
    D_ASSERT(0u < value);
    return size_mul_or_max<3_uz>(value);
}

[[nodiscard]] constexpr size_t optimal_memory_limit(size_t value) noexcept
{
    constexpr auto mul = optimal_memory_growth(optimal_memory_growth(1_uz));
    constexpr auto add = 1_uz;

    constexpr auto overflow = size_overflow_maxi / mul;
    static_assert(add <= (size_overflow_maxi - overflow * mul));

    D_ASSERT(0u < value);
    if (value <= overflow) [[likely]]
    {
        return value * mul + add;
    }

    return size_overflow_maxi;
}

constexpr size_t small_vector_min_static_size_bytes_v = size_align<sizeof(buffer_void)>(std::max(small_vector_exceptions::msg_max_len, sizeof(buffer_void)));

template<class T>
constexpr size_t small_vector_default_static_size_v = ceil_div(small_vector_min_static_size_bytes_v, sizeof(T));

template
<
    class T,
    size_t N = small_vector_default_static_size_v<T>,
    class Buffer = buffer<T>
>
class small_vector
{
    using temp_vector_type = private_detail_small_vector::temp_vector<T, Buffer>;

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
    using size_type = typename buffer_type::size_type;
    using view_type = const_span_type;
    using null_type = nullmem_t;

    static_assert(std::is_same_v<size_t, size_type>);
    static_assert(std::is_nothrow_default_constructible_v<buffer_type>);
    static_assert(std::is_nothrow_constructible_v<buffer_type, size_type>);
    static_assert(std::is_nothrow_move_constructible_v<buffer_type>);
    static_assert(std::is_nothrow_move_assignable_v<buffer_type>);
    static_assert(!std::is_copy_constructible_v<buffer_type>);
    static_assert(!std::is_copy_assignable_v<buffer_type>);

    static constexpr size_type static_size{ N };
    static constexpr auto min_capacity_limit = optimal_memory_limit(static_size);

    static_assert(0u < static_size);
    static_assert(static_size < min_capacity_limit);

    template<class FnBadAllocException>
    using fn_bad_alloc_exception_invocable = std::disjunction<
        std::is_nothrow_invocable<FnBadAllocException>,
        std::is_nothrow_invocable<FnBadAllocException, pointer, size_type>
    >;

    template<class FnBadAllocException>
    static constexpr bool fn_bad_alloc_exception_invocable_v = fn_bad_alloc_exception_invocable<FnBadAllocException>::value;

    constexpr small_vector() noexcept
        : data_{ static_ }
        , size_{}
    {}

    constexpr small_vector(null_type) noexcept
        : small_vector{}
    {}

    constexpr small_vector(const small_vector& right) noexcept
        : small_vector
        {
            static_cast<const_span_type>(right),
            small_vector_exceptions::accept_bad_alloc
        }
    {}

    template<class FnBadAllocException, std::enable_if_t<fn_bad_alloc_exception_invocable_v<FnBadAllocException>, int> = 0>
    constexpr small_vector(const_span_type right, FnBadAllocException&& accept_bad_alloc) noexcept
        : small_vector{
            memory_overwrite_construct,
            right.size(),
            [right] (pointer data, size_t data_capacity) noexcept {
                D_ASSERT(right.size() <= data_capacity);
                copynew(right, data);
            },
            std::forward<FnBadAllocException>(accept_bad_alloc)
        }
    {}

    template<class FnOverwrite, class FnBadAllocException, std::enable_if_t<std::conjunction_v<
        std::is_nothrow_invocable<FnOverwrite, pointer, size_type>,
        fn_bad_alloc_exception_invocable<FnBadAllocException>
    >, int> = 0>
    constexpr small_vector(memory_overwrite_construct_t, size_type new_size, FnOverwrite&& overwrite_op, FnBadAllocException&& accept_bad_alloc) noexcept
        : small_vector{}
    {
        size_t new_capacity{ static_size };

        if (new_size > static_size)
        {
            buffer_type temp{ new_size };
            if (!temp) [[unlikely]]
            {
                if constexpr (std::is_invocable_v<FnBadAllocException, pointer, size_type>)
                {
                    if constexpr (std::is_integral_v<std::invoke_result_t<FnBadAllocException, pointer, size_type>>)
                    {
                        const auto error_size = narrow<size_type>(std::invoke(std::forward<FnBadAllocException>(accept_bad_alloc), static_, static_size));
                        size_ = std::min(error_size, static_size);
                    }
                    else
                    {
                        std::invoke(std::forward<FnBadAllocException>(accept_bad_alloc), static_, static_size);
                    }
                }
                else
                {
                    std::invoke(std::forward<FnBadAllocException>(accept_bad_alloc));
                }

                return;
            }

            new_capacity = std::size(temp);
            _dynamic_construct_buffer(std::move(temp));
        }

        if constexpr (std::is_integral_v<std::invoke_result_t<FnOverwrite, pointer, size_type>>)
        {
            const auto overwrite_size = narrow<size_type>(std::invoke(std::forward<FnOverwrite>(overwrite_op), data_, new_capacity));
            size_ = std::min(overwrite_size, new_capacity);
        }
        else
        {
            std::invoke(std::forward<FnOverwrite>(overwrite_op), data_, new_capacity);
            size_ = new_size;
        }
    }

    constexpr small_vector(memory_construct_t, buffer_type&& mem) noexcept
        : small_vector{}
    {
        _dynamic_construct_buffer(std::move(mem));
    }

    constexpr small_vector(small_vector&& right) noexcept
        : small_vector{}
    {
        if (right.is_static())
        {
            movenew(make_span(right), static_);
            size_ = right.size();
        }
        else
        {
            _dynamic_construct(std::move(right));
        }
    }

    constexpr small_vector& operator = (null_type) noexcept
    {
        clear();
        return *this;
    }

    constexpr small_vector& operator = (const small_vector& right) noexcept
    {
        if (this != std::addressof(right)) [[likely]]
        {
            if (!try_assign(right)) [[unlikely]]
            {
                std::invoke(small_vector_exceptions::accept_bad_alloc);
            }
        }

        return *this;
    }

    constexpr small_vector& operator = (small_vector&& right) noexcept
    {
        if (this != std::addressof(right)) [[likely]]
        {
            if (right.is_static())
            {
                clear();
                movenew(make_span(right), data_);
                size_ = right.size();
            }
            else
            {
                if (is_static())
                {
                    _destroys();
                    _dynamic_construct(std::move(right));
                }
                else
                {
                    dynamic_.swap(right.dynamic_);
                    std::swap(data_, right.data_);
                    std::swap(size_, right.size_);
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
    bool try_assign(const const_span_type right) noexcept
    {
        if (right.size() > capacity())
        {
            buffer_type temp{ right.size() };
            if (!temp) [[unlikely]]
            {
                return false;
            }

            copynew(right, std::data(temp));
            _destroys();
            _set_buffer(std::move(temp));
        }
        else
        {
            _clear();
            _collect(right.size());
            copynew(right, data_);
        }

        size_ = right.size();

        return true;
    }

    constexpr pointer attach_buffer(buffer_type&& mem) noexcept
    {
        D_ASSERT(std::size(mem) >= size());

        const span_type sp{ data_, size() };
        const auto result = movenew(sp, std::data(mem));
        std::destroy_n(sp.data(), sp.size());
        _set_buffer(std::move(mem));
        return result;
    }

    [[nodiscard]]
    constexpr buffer_type release_buffer() noexcept
    {
        buffer_type temp{};

        if (is_dynamic())
        {
            _clear();
            _set_static();
            temp = std::move(dynamic_);
            _destroy_nullmem();
        }

        return temp;
    }

    template<class... Args>
    [[nodiscard]] constexpr const_iterator try_emplace(const_iterator position, Args&&... args) noexcept
    {
        D_ASSERT(position >= cbegin());
        D_ASSERT(position <= cend());

        const auto mut_postion = const_cast<pointer>(position);
        const auto size = small_vector::size();
        const auto new_size = size_add<1u>(size);
        const auto old_capacity = small_vector::capacity();
        const auto first = data_;
        const auto last = first + size;

        if (size < old_capacity) [[likely]]
        {
            using private_detail_small_vector::back_move;

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
                        ::move_construct_at(last, std::move(*p_back));
                        back_move(mut_postion, p_back);
                    }
                    ::destroy_at(mut_postion);
                }
            }

            ::construct_at(mut_postion, std::forward<Args>(args)...);
            size_ = new_size;
            return position;
        }

        if (temp_vector_type temp{ optimal_memory_growth(old_capacity) }) [[likely]]
        {
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
        D_ASSERT(first >= cbegin());
        D_ASSERT(last <= cend());

        const auto data = data_;
        const auto size = small_vector::size();
        const auto removed_data = std::move(const_cast<pointer>(last), data + size, const_cast<pointer>(first));
        const auto new_size = u_distance(data, removed_data);
        const auto count_of_erased = u_distance(new_size, size);
        size_ = new_size;
        std::destroy_n(removed_data, count_of_erased);
        _collect(new_size);

        return count_of_erased;
    }

    [[nodiscard]]
    constexpr bool try_reserve(size_type new_capacity) noexcept
    {
        return (new_capacity <= capacity())
            || _try_reallocate(new_capacity);
    }

    void reserve(size_type new_capacity) noexcept
    {
        D_CHECK(try_reserve(new_capacity));
    }

    [[nodiscard]]
    constexpr bool try_shrink_to_fit() noexcept
    {
        bool ok{ true };

        if (is_dynamic())
        {
            const auto capacity = std::size(dynamic_);
            const auto size = small_vector::size();

            if (capacity > size)
            {
                if (size <= static_size)
                {
                    _switch_to_static();
                }
                else
                {
                    if (capacity > buffer_type::good_size(size))
                    {
                        ok = _try_reallocate(size);
                    }
                }
            }
        }

        return ok;
    }

    void shrink_to_fit() noexcept
    {
        D_CHECK(try_shrink_to_fit());
    }

    template<class... Args>
    [[nodiscard]] constexpr pointer try_emplace_back(Args&&... args) noexcept
    {
        const auto size = small_vector::size();
        const auto new_size = size_add<1u>(size);
        const auto old_capacity = capacity();

        if (size < old_capacity) [[likely]]
        {
            const auto last = data_ + size;
            ::construct_at(last, std::forward<Args>(args)...);
            size_ = new_size;
            return last;
        }

        if (buffer_type temp{ optimal_memory_growth(old_capacity) }) [[likely]]
        {
            const auto new_last = attach_buffer(std::move(temp));
            ::construct_at(new_last, std::forward<Args>(args)...);
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
        D_ASSERT(0u < size_);
        ::destroy_at(--size_ + data_);
        _collect();
    }

    void erase(const_iterator position) noexcept
    {
        D_ASSERT(position < cend());
        erase(position, std::next(position));
    }

    constexpr void clear() noexcept
    {
        _clear();
        _collect(0u);
    }

    [[nodiscard]]
    constexpr size_type capacity() const noexcept
    {
        return is_static() ? static_size : std::size(dynamic_);
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
    constexpr const_pointer cdata() const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept
    {
        return cdata();
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
        D_ASSERT(0u < size_);
        return *cdata();
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        D_ASSERT(0u < size_);
        return cdata()[size_ - 1u];
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
        return cvalue(index);
    }

    [[nodiscard]]
    constexpr reference operator[](size_type index) noexcept
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
        const auto first = cdata();
        const auto data_is_static = (static_ == first);
        D_ASSERT(data_is_static || _buffer_is_valid_for(first, dynamic_));
        return data_is_static;
    }

    [[nodiscard]]
    constexpr bool is_dynamic() const noexcept
    {
        return !is_static();
    }

    constexpr ~small_vector() noexcept
    {
        _destroys();

        if (is_dynamic())
        {
            _destroy_dynamic();
        }
    }

private:
    constexpr void _set_buffer(buffer_type&& mem) noexcept
    {
        if (is_static())
        {
            _construct_buffer(std::move(mem));
        }
        else
        {
            _assignment_buffer(std::move(mem));
        }

        _set_dynamic();
    }

    constexpr void _set_static() noexcept
    {
        D_ASSERT(is_dynamic());
        data_ = static_;
    }

    constexpr void _set_dynamic() noexcept
    {
        D_ASSERT(_buffer_is_valid(dynamic_));
        D_ASSERT(cdata() != ::cdata(dynamic_));
        data_ = std::data(dynamic_);
    }

    constexpr void _construct_buffer(buffer_type&& mem) noexcept
    {
        D_ASSERT(_buffer_is_valid(mem));
        D_ASSERT(is_static());
        ::move_construct_at(std::addressof(dynamic_), std::move(mem));
    }

    constexpr void _assignment_buffer(buffer_type&& mem) noexcept
    {
        D_ASSERT(_buffer_is_valid(mem));
        D_ASSERT(is_dynamic());
        D_ASSERT(cdata() != ::cdata(mem));
        dynamic_ = std::move(mem);
    }

    constexpr void _dynamic_construct_buffer(buffer_type&& mem) noexcept
    {
        D_ASSERT(is_static());
        _construct_buffer(std::move(mem));
        _set_dynamic();
    }

    constexpr void _dynamic_construct(small_vector&& right) noexcept
    {
        D_ASSERT(is_static());
        right._set_static();
        _dynamic_construct_buffer(std::move(right.dynamic_));
        size_ = std::exchange(right.size_, {});
        right._destroy_nullmem();
    }

    constexpr void _destroys() const noexcept
    {
        D_ASSERT((static_ == cdata()) || _buffer_is_valid_for(cdata(), dynamic_));
        std::destroy_n(data_, size_);
    }

    constexpr void _clear() noexcept
    {
        _destroys();
        size_ = {};
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

        _set_static();
        const auto size = std::exchange(size_, {});
        const temp_vector_type temp{ std::move(dynamic_), size };
        _destroy_nullmem();
        movenew(temp.begin(), temp.end(), static_);
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
                : optimal_memory_limit(expected_capacity);

            const auto current_capacity = std::size(dynamic_);

            if (current_capacity >= capacity_limit) [[unlikely]]
            {
                const auto buffer_size_limit = size_add_or_max<1_uz>(
                    buffer_type::good_size(capacity_limit)
                );

                if (current_capacity >= buffer_size_limit)
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
        }

        return ok;
    }

    constexpr void _collect(size_type expected_capacity) noexcept
    {
        D_CHECK(_try_collect(expected_capacity));
    }

    constexpr void _collect() noexcept
    {
        _collect(size_);
    }

    constexpr void _destroy_nullmem() const noexcept
    {
        D_ASSERT(_buffer_is_null(dynamic_));

        if constexpr (!std::is_base_of_v<buffer_void, buffer_type>)
        {
            _destroy_dynamic();
        }
    }

    constexpr void _destroy_dynamic() const noexcept
    {
        ::destroy(dynamic_);
    }

    static constexpr bool _buffer_size_is_valid(const buffer_type& mem) noexcept
    {
        return static_size < std::size(mem);
    }

    static constexpr bool _buffer_is_valid(const buffer_type& mem) noexcept
    {
        return (::cdata(mem)) && _buffer_size_is_valid(mem);
    }

    static constexpr bool _buffer_is_valid_for(const_pointer data, const buffer_type& mem) noexcept
    {
        return (data == ::cdata(mem)) && _buffer_size_is_valid(mem);
    }

    static constexpr bool _buffer_is_null(const buffer_type& mem) noexcept
    {
        return !::cdata(mem) && !std::size(mem);
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

static_assert(small_size_mini == sizeof(small_vector<char>));
static_assert(small_size_mini == sizeof(small_vector<std::pair<void*, void*> >));

D_WARNING_POP