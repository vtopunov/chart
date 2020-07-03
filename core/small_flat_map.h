#pragma once

#include <core/small_vector.h>

template<class Key, class Value>
struct key_value
{
    using key_type = Key;
    using value_type = Value;
    using key_view = key_type;

    key_type key;
    value_type value;
};

template<class iterator>
struct iterator_range
{
    iterator first;
    iterator last;
};

template<class Key, class Value>
struct key_value_less
{
    using key_value_type = key_value<Key, Value>;
    using key_view = typename key_value_type::key_view;

    static constexpr key_view key(const key_value<Key, Value>& key_value) noexcept
    {
        return key_value.key;
    }

    static constexpr key_view key(key_view key) noexcept
    {
        return key;
    }

    template<class LT, class RT>
    constexpr bool operator () (const LT& left, const RT& right) const noexcept
    {
        return key(left) < key(right);
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

    for (; it != range.last; ++it)
    {
        if (it->key != key)
            break;
    }

    return { range.first, it };
}

template<class K, class T, size_t N>
class small_flat_map
{
public:
    static constexpr size_t small_size = N;

    using key_type = K;
    using mapped_type = T;
    using key_value_type = key_value<key_type, mapped_type>;
    using key_view = typename key_value_type::key_view;
    using less_type = key_value_less<key_type, mapped_type>;

    using dynarray_type = small_vector<key_value_type, small_size>;

    using pointer = typename dynarray_type::pointer;
    using const_pointer = typename dynarray_type::const_pointer;
    using reference = typename dynarray_type::reference;
    using const_reference = typename dynarray_type::const_reference;
    using iterator = typename dynarray_type::iterator;
    using const_iterator = typename dynarray_type::const_iterator;

    constexpr iterator_range<const_iterator> lower_bound(key_view key) const noexcept
    {
        const auto const_end = cend();
        return { std::lower_bound(cbegin(), const_end, key, less_type{}), const_end };
    }

    constexpr iterator_range<const_iterator> upper_bound(key_view key) const noexcept
    {
        const auto const_end = cend();
        return { std::upper_bound(cbegin(), const_end, key, less_type{}), const_end };
    }

    constexpr iterator_range<const_iterator> bound(key_view key) const noexcept
    {
        return left_by_key(lower_bound(key), key);
    }

    constexpr const_iterator find(key_view key) const noexcept
    {
        const auto range = lower_bound(key);
        return starts_with_key(range, key) ? range.first : range.last;
    }

    constexpr size_t count(key_view key) const noexcept
    {
        const auto range = bound(key);
        return narrow_cast<size_t>(range.last - range.first);
    }

    constexpr bool contains(key_view key) const noexcept
    {
        return starts_with_key(lower_bound(key), key);
    }

    std::pair<const_iterator, bool> insert(key_value_type value) noexcept
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
        return insert(key_value_type{ std::move(key), std::move(value) });
    }

    const_iterator force_insert(key_value_type value) noexcept
    {
        const auto position = upper_bound(value.key).first;
        return unsafe_force_insert_hint(position, std::move(value));
    }

    const_iterator force_insert(key_type key, mapped_type value) noexcept
    {
        return force_insert(key_value_type{ std::move(key), std::move(value) });
    }

    size_t erase(const_iterator first, const_iterator last) noexcept
    {
        return data_.erase(first, last);
    }

    size_t erase(iterator_range<const_iterator> range) noexcept
    {
        return erase(range.first, range.last);
    }

    size_t erase(key_view key) noexcept
    {
        return erase(bound(key));
    }

    void erase(const_iterator position) noexcept
    {
        data_.erase(position);
    }

    void clear() noexcept
    {
        data_.clear();
    }

    const_iterator unsafe_force_insert_hint(const_iterator position, key_value_type item) noexcept
    {
        return data_.insert(position, std::move(item));
    }

    void shrink_to_fit() noexcept
    {
        data_.shrink_to_fit();
    }

    constexpr size_t size() const noexcept
    {
        return data_.size();
    }

    constexpr size_t capacity() const noexcept
    {
        return data_.capacity();
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

    constexpr bool is_small() const noexcept
    {
        return data_.is_small();
    }

private:
    dynarray_type data_;
};
