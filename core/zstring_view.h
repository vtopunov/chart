#pragma once

#include <string_view>
#include <string>
#include <compare>

template<class T>
inline constexpr T empty_c_string[] = { T{} };

class zstring_view // null terminated string_view
{
public:
    using string_view = std::string_view;
    using traits_type = typename string_view::traits_type;
    using value_type = typename string_view::value_type;
    using const_pointer = typename string_view::const_pointer;
    using const_reference = typename string_view::const_reference;
    using const_iterator = typename string_view::const_iterator;
    using const_reverse_iterator = typename string_view::const_reverse_iterator;;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    constexpr zstring_view() noexcept
        : string_{ empty_c_string<value_type> }
    {}

    constexpr zstring_view(const_pointer c_string) noexcept
        : string_{ c_string }
    {}

    template<class Alloc>
    zstring_view(const std::basic_string<value_type, traits_type, Alloc>& string) noexcept
        : string_{ string.c_str(), string.size() }
    {}

    constexpr operator string_view() const noexcept
    {
        return string_;
    }

    constexpr const_pointer c_str() const noexcept
    {
        return string_.data();
    }

    constexpr const_pointer data() const noexcept
    {
        return string_.data();
    }

    constexpr size_type size() const noexcept
    {
        return string_.size();
    }

    constexpr const_iterator begin() const noexcept
    {
        return string_.begin();
    }

    constexpr const_iterator end() const noexcept
    {
        return string_.end();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return string_.begin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return string_.end();
    }

    constexpr auto operator<=>(const zstring_view&) const noexcept = default;

private:
    string_view string_;
};