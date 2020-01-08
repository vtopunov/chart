#pragma once

#include <vector>
#include <iterator>

#include <core/util.h>
#include <core/span.h>

template<class iterator>
struct optional_iterator
{
    iterator position;
    bool has_value;

    constexpr iterator position_or(iterator other) const noexcept
    {
        return (has_value) ? position : other;
    }

    explicit constexpr operator bool() const noexcept
    {
        return has_value;
    }

    constexpr iterator operator -> () const noexcept
    {
        assert(has_value);
        return position;
    }

    constexpr decltype(auto) operator * () const noexcept
    {
        assert(has_value);
        return *position;
    }
};

template<class K, class T, size_t N>
class small_flat_map
{
public:
    static constexpr size_t static_size = N;

    using key_type = K;
    using key_view = key_type;
    using mapped_type = T;

    struct value_type
    {
        key_type key;
        mapped_type value;

        constexpr operator key_view () const noexcept
        {
            return key;
        }
    };

    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using optional_item = optional_iterator<iterator>;
    using const_optional_item = optional_iterator<const_iterator>;

    static constexpr auto less = [](key_view left, key_view right) noexcept
    {
        return left < right;
    };

    static constexpr auto eq = [](key_view left, key_view right) noexcept
    {
        return left == right;
    };

    constexpr small_flat_map() noexcept {}

    const_iterator find(key_view key) const noexcept
    {
        return mutable_this()->find(key);
    }

    const_optional_item item(key_view key) const noexcept
    {
        const auto result = mutable_this()->item(key);
        return { as_const_pointer(result.position), result.has_value };
    }

    iterator find(key_view key) noexcept
    {
        return item(key).position_or(end());
    }

    optional_item item(key_view key) noexcept
    {
        const auto position = _lower_bound(key);
        return { position, position < cend() && eq(*position, key) };
    }

    span<value_type> items(key_view key) noexcept
    {
        const auto postion = _lower_bound(key);
        return { postion, count(postion, key) };
    }

    span<const value_type> items(key_view key) const noexcept
    {
        return mutable_this()->items(key).cspan();
    }

    constexpr size_t count(const_iterator position, key_view key) const noexcept
    {
        assert(_is_position(position));

        const auto position0 = position;
        for (const auto end = cend(); position != end && eq(position->key, key); ++position) // c++20 constexpr count_if
        {
        }

        return narrow_cast<size_t>(position - position0);
    }

    constexpr size_t count(const_iterator position) const noexcept
    {
        assert(_is_position(position));
        return (position != cend()) ? (count(std::next(position), position->key) + 1_z) : 0_z;
    }

    size_t count(key_view key) const noexcept
    {
        return items(key).size();
    }

    bool contains(key_view key) const noexcept
    {
        return item(key).has_value;
    }

    std::pair<iterator, bool> insert(value_type value) noexcept
    {
        const auto position = item(value.key);
        if (position)
        {
            return { position.position, false };
        }

        const auto result =
            force_insert_hint(position.position, std::move(value));

        return { result, true };
    }

    std::pair<iterator, bool> insert(key_type key, mapped_type value) noexcept
    {
        return insert(value_type{ std::move(key), std::move(value) });
    }

    iterator force_insert(value_type new_item) noexcept
    {
        const auto position = _upper_bound(new_item);
        return force_insert_hint(position, std::move(new_item));
    }

    iterator force_insert(key_type key, mapped_type value) noexcept
    {
        return force_insert(value_type{ std::move(key), std::move(value) });
    }

    size_t erase(key_view key) noexcept
    {
        const auto values = items(key);
        erase(values.begin(), values.end());
        return values.size();
    }

    void erase(iterator position) noexcept
    {
        erase(position, std::next(position));
    }

    void erase(iterator begin, iterator end) noexcept
    {
        assert(end >= begin);
        assert(begin >= data_.cbegin());
        assert(end <= data_.cend());

        if (is_static())
        {
            std::destroy(begin, end);
            std::uninitialized_move(end, data_.end(), begin);
            data_ = data_.remove_suffix(narrow_cast<size_t>(std::distance(begin, end)));
            return;
        }

        dynamic_.erase(
            _dynamic_position(_position_index(begin)),
            _dynamic_position(_position_index(end))
        );

        data_ = dynamic_;
    }

    iterator force_insert_hint(iterator position, value_type item)
    {
        const auto position_index = _position_index(position);

        if (is_static())
        {
            if (size() < static_size)
            {
                std::move_backward(position, data_.end(), std::uninitialized_default_construct_n(data_.end(), 1_z));
                *position = std::move(item);
                data_ = data_.extend_suffix(1_z);
                return position;
            }

            _switch_to_dynamic();
        }

        const auto result = dynamic_.insert(_dynamic_position(position_index), std::move(item));
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

    constexpr size_t size() const noexcept { return data_.size(); }

    constexpr size_t capacity() const noexcept { return (is_static()) ? static_size : dynamic_.capacity(); }

    constexpr const_iterator data() const noexcept { return data_.data(); }

    constexpr iterator begin() noexcept { return data_.begin(); }

    constexpr iterator end() noexcept { return data_.end(); }

    constexpr const_iterator begin() const noexcept { return data_.cbegin(); }

    constexpr const_iterator end() const noexcept { return data_.cend(); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }

    constexpr const_iterator cend() const noexcept { return end(); }

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

    constexpr small_flat_map* mutable_this() const noexcept
    {
        return as_mutable_pointer(this);
    }

    void _destroy_static() noexcept
    {
        std::destroy(begin(), end());
    }

    void _destroy_dynamic() noexcept
    {
        std::destroy_at(&(dynamic_));
    }

    void _switch_to_static() noexcept
    {
        dynarray_type dynamic{ std::move(dynamic_) };
        _destroy_dynamic();
        data_ = { std::data(static_), dynamic.size() };
        std::uninitialized_move(dynamic.begin(), dynamic.end(), begin());
    }

    void _switch_to_dynamic() noexcept
    {
        dynarray_type dynamic;
        dynamic.reserve(2 * size());
        dynamic.assign(std::make_move_iterator(begin()), std::make_move_iterator(end()));
        _destroy_static();
        new (&dynamic_) dynarray_type(std::move(dynamic));
        data_ = dynamic_;
    }

    iterator _lower_bound(key_view key) noexcept
    {
        return std::lower_bound(begin(), end(), key, less);
    }

    iterator _upper_bound(key_view key) noexcept
    {
        return std::upper_bound(begin(), end(), key, less);
    }

    constexpr bool _is_position(const_iterator position) const noexcept
    {
        return position >= cbegin() && position <= cend();
    }

    constexpr size_t _position_index(const_iterator position) const noexcept
    {
        assert(_is_position(position));
        return narrow_cast<size_t>(position - cbegin());
    }

    const_iterator_dynarray _dynamic_position(size_t position) const noexcept
    {
        assert(!is_static() && position <= dynamic_.size());
        return dynamic_.cbegin() + position;
    }

private:
    union
    {
        value_type static_[static_size];
        dynarray_type dynamic_;
    };
    span<value_type> data_{ std::data(static_), 0_z };
};
