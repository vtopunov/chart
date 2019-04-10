#pragma once

#include <cmath>
#include <iterator>

#include "util.h"

template<class T>
struct num_sequence
{
    typedef T value_type;
    typedef size_t size_type;

    value_type first;
    value_type last;
    size_type count;

    constexpr value_type operator [](size_type i) const noexcept
    {
        return value(i);
    }

    constexpr value_type value(size_type i) const noexcept
    {
        return ((count - i) * first + i * last) / count;
    }

    size_type index(value_type value) const noexcept
    {
        const auto index = std::round(count * (value - first) / (last - first));
        return std::clamp(index, 0_z, count - 1_z);
    }

    constexpr value_type round(value_type value) const noexcept
    {
        return num_sequence::value(index(value));
    }

    constexpr value_type front() const noexcept
    {
        return first;
    }

    constexpr value_type back() const noexcept
    {
        return last;
    }

    constexpr size_type size() const noexcept
    {
        return count;
    }

    constexpr bool empty() const noexcept
    {
        return !count;
    }

    struct const_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = num_sequence::value_type;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const value_type*;
        using reference         = const value_type &;

        num_sequence sequence;
        difference_type position;

        constexpr value_type operator*() const noexcept
        {
            return sequence[position];
        }

        const_iterator& operator++() noexcept
        {
            ++position;
            return *this;
        }

        const_iterator operator++(int) noexcept
        {
            const_iterator tmp(*this);
            ++* this;
            return tmp;
        }

        const_iterator& operator --() noexcept
        {
            --position;
            return *this;
        }

        const_iterator operator--(int) noexcept
        {
            const_iterator tmp(*this);
            --* this;
            return tmp;
        }

        const_iterator& operator+=(difference_type n) noexcept
        {
            position += n;
            return *this;
        }

        const_iterator& operator-=(difference_type n) noexcept
        {
            position -= n;
            return *this;
        }

        constexpr const_iterator operator + (difference_type n) const noexcept
        {
            return const_iterator(*this) += n;
        }

        constexpr const_iterator operator - (difference_type n) const noexcept
        {
            return const_iterator(*this) -= n;
        }

        constexpr difference_type operator - (const const_iterator& r) const noexcept
        {
            return position - r.position;
        }

        constexpr value_type operator[] (difference_type n) const noexcept
        {
            return *(*this + n);
        }

        constexpr bool operator==(const const_iterator & r) const noexcept
        {
            return position == r.position;
        }

        constexpr bool operator!=(const const_iterator & r) const noexcept
        {
            return position != r.position;
        }

        constexpr bool operator< (const const_iterator & r) const noexcept
        {
            return position < r.position;
        }

        constexpr bool operator> (const const_iterator & r) const noexcept
        {
            return position > r.position;
        }

        constexpr bool operator<=(const const_iterator & r) const noexcept
        {
            return position <= r.position;
        }

        constexpr bool operator>=(const const_iterator & r) const noexcept
        {
            return position >= r.position;
        }
    };

    constexpr const_iterator begin() const noexcept
    {
        return { *this, 0 };
    }

    constexpr const_iterator end() const noexcept
    {
        return { *this, narrow_cast<ptrdiff_t>(count) };
    }
};
