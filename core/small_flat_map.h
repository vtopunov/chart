#pragma once

#include <vector>
#include <algorithm>
#include <ranges>

#include <core/defs.h>
#include <core/span.h>

template<class Key, class Value>
struct key_value
{
    using key_type = Key;
    using value_type = Value;
    using key_view = key_type;

    key_type key;
    value_type value;
};

template<class Iterator>
struct iterator_range
{
    Iterator first;
    Iterator last;
};

template<class Key, class Value>
struct less_by_key_function
{
    template<class KeyView>
    constexpr bool operator () (const key_value<Key, Value>& key_value, KeyView key) noexcept
    {
        return key_value.key < key;
    }
};

template<class Key, class Value>
using const_key_value_range_t = iterator_range<const key_value<Key, Value>*>;

template<class Key, class Value, class KeyView>
constexpr bool starts_with_key(const const_key_value_range_t<Key, Value>& range, KeyView key) noexcept
{
    return range.first != range.last && range.first->key == key;
}

template<class Key, class Value, class KeyView>
constexpr const_key_value_range_t<Key, Value> left_by_key(const_key_value_range_t<Key, Value> range, KeyView key) noexcept
{
    auto it = range.first;

    for (; it != range.last && it->key == key; ++it)
    {}

    return { range.first, it };
}

template<class K, class T, size_t N>
class small_flat_map
{
public:
    static constexpr size_t static_size = N;

    using key_type = K;
    using mapped_type = T;
    using value_type = key_value<key_type, mapped_type>;
    using key_view = typename value_type::key_view;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;

    constexpr small_flat_map() noexcept
        : data_{ std::data(static_), 0u }
    {}

    constexpr size_t position_to_index(const_iterator position) const noexcept
    {
        D_ASSERT(_is_position(position));
        return narrow_cast<size_t>(position - cbegin());
    }

    static constexpr less_by_key_function<key_type, mapped_type> less_by_key{};

    iterator_range<const_iterator> lower_bound(key_view key) const noexcept // c++20 constexpr
    {
        const auto const_end = cend();
        return { std::lower_bound(cbegin(), const_end, key, less_by_key), const_end };
    }

    iterator_range<const_iterator> upper_bound(key_view key) const noexcept // c++20 constexpr
    {
        const auto const_end = cend();
        return { std::upper_bound(cbegin(), const_end, key, less_by_key), const_end };
    }

    const_iterator find(key_view key) const noexcept
    {
        const auto range = lower_bound(key);
        return starts_with_key(range, key) ? range.first : range.last;
    }

    size_t count(key_view key) const noexcept
    {
        const auto range = left_by_key(lower_bound(key), key);
        return narrow_cast<size_t>(range.last - range.first);
    }

    bool contains(key_view key) const noexcept
    {
        return starts_with_key(lower_bound(key), key);
    }

    std::pair<const_iterator, bool> insert(value_type value) noexcept
    {
        const auto lb = lower_bound(value.key);
        const auto already_contained = starts_with_key(lb, value.key);
        return
        {
            (
                already_contained
                ? lb.first
                : unsafe_force_insert_hint(lb.first, std::move(value))
            ),
            !already_contained
        };
    }

    std::pair<const_iterator, bool> insert(key_type key, mapped_type value) noexcept
    {
        return insert(value_type{ std::move(key), std::move(value) });
    }

    const_iterator force_insert(value_type value) noexcept
    {
        const auto position = upper_bound(value.key).first;
        return unsafe_force_insert_hint(position, std::move(value));
    }

    const_iterator force_insert(key_type key, mapped_type value) noexcept
    {
        return force_insert(value_type{ std::move(key), std::move(value) });
    }

    size_t erase(const_iterator first, const_iterator last) noexcept
    {
        D_ASSERT(last >= first);
        D_ASSERT(first >= data_.cbegin());
        D_ASSERT(last <= data_.cend());

        const auto new_first = std::move(const_cast<iterator>(last), data_.end(), const_cast<iterator>(first));
        const auto new_last = data_.end();
        const auto count_of_erased = narrow_cast<size_t>(last - first);
        const auto new_size = data_.size() - count_of_erased;
        data_ = data_.left(new_size);

        if (is_static())
        {
            std::destroy(new_first, new_last);
        }
        else
        {
            dynamic_.resize(new_size);
            D_ASSERT(data_.data() == dynamic_.data());
            D_ASSERT(data_.size() == dynamic_.size());
        }

        return count_of_erased;
    }

    size_t erase(iterator_range<const_iterator> range) noexcept
    {
        return erase(range.first, range.last);
    }

    size_t erase(key_view key) noexcept
    {
        const auto range = left_by_key(std::as_const(*this).lower_bound(key), key);
        return erase(range.first, range.last);
    }

    void erase(const_iterator position) noexcept
    {
        erase(position, std::next(position));
    }

    void clear() noexcept
    {
        erase(cbegin(), cend());
    }

    const_iterator unsafe_force_insert_hint(const_iterator position, value_type item)
    {
        const auto index = position_to_index(position);

        if (is_static())
        {
            if (size() < static_size)
            {
                {
                    const auto mutable_postion = const_cast<iterator>(position);
                    std::move_backward(mutable_postion, data_.end(), std::uninitialized_default_construct_n(data_.end(), 1));
                    *mutable_postion = std::move(item);
                }
                data_ = data_.extend_suffix(1u);
                return position;
            }

            _switch_to_dynamic();
        }

        const auto result = dynamic_.insert(_dynamic_position(index), std::move(item));
        data_ = dynamic_;
        return &(*result);
    }

    void shrink_to_fit() noexcept
    {
        if (!is_static())
        {
            if (size() <= static_size)
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

    constexpr bool empty() const noexcept
    {
        return data_.empty();
    }

    constexpr size_t size() const noexcept
    {
        return data_.size();
    }

    constexpr size_t capacity() const noexcept
    {
        return (is_static()) ? static_size : dynamic_.capacity();
    }

    constexpr const_iterator data() const noexcept
    {
        return data_.data();
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

    constexpr bool is_static() const noexcept
    {
        return cbegin() == std::cbegin(static_);
    }

    ~small_flat_map() noexcept
    {
        if (is_static())
        {
            _destroy_static();
        }
        else
        {
            _destroy_dynamic();
        }
    }

private:
    using dynarray_type = std::vector<value_type>;
    using const_iterator_dynarray = typename dynarray_type::const_iterator;

    void _destroy_static() noexcept
    {
        const auto data = data_;
        data_ = { std::data(static_), 0u };
        std::destroy(data.begin(), data.end());
    }

    void _destroy_dynamic() noexcept
    {
        data_ = { std::data(static_), 0u };
        std::destroy_at(&(dynamic_));
    }

    void _switch_to_static() noexcept
    {
        D_ASSERT(!is_static() && dynamic_.size() <= static_size);
        dynarray_type dynamic{ std::move(dynamic_) };
        _destroy_dynamic();
        data_ = { std::data(static_), dynamic.size() };
        std::uninitialized_move(dynamic.begin(), dynamic.end(), static_);
    }

    void _switch_to_dynamic() noexcept
    {
        D_ASSERT(is_static());
        dynarray_type dynamic;
        dynamic.reserve(3u * static_size);
        dynamic.assign(std::make_move_iterator(begin()), std::make_move_iterator(end()));
        _destroy_static();
        new (&dynamic_) dynarray_type{ std::move(dynamic) };
        data_ = dynamic_;
    }

    constexpr bool _is_position(const_iterator position) const noexcept
    {
        return position >= cbegin() && position <= cend();
    }

    const_iterator_dynarray _dynamic_position(size_t position) const noexcept
    {
        D_ASSERT(!is_static() && position <= dynamic_.size());
        return dynamic_.cbegin() + position;
    }

private:
    union
    {
        value_type static_[static_size];
        dynarray_type dynamic_;
    };
    span<value_type> data_;
};
