#pragma once

#include <string_view>
#include <compare>

#include <core/assert.h>

template<class T>
inline constexpr T empty_c_string_v[] = { T{} };

template <class, class = void>
struct is_string : std::false_type
{};

template <class T>
struct is_string<T, std::void_t<decltype(std::declval<T>().c_str()), decltype(std::size(std::declval<T&>()))>>
    : std::true_type
{};

template<class T>
inline constexpr bool is_string_v = is_string<T>::value;

template<class T>
class basic_zstring_view;

template <class T>
struct is_zstring_view : std::false_type
{};

template <class T>
struct is_zstring_view<basic_zstring_view<T>> : std::true_type
{};

template <class T>
struct is_zstring_view<const T> : is_zstring_view<T>
{};

template<class T>
inline constexpr bool is_zstring_view_v = is_zstring_view<T>::value;

struct c_str_construct_t
{};

inline constexpr c_str_construct_t c_str_construct{};

template<class T>
constexpr bool is_null_terminated(const T* string, size_t size) noexcept
{
    while (string[size])
    {
        if (!size)
        {
            return false;
        }

        --size;
    }

    return true;
}

template<class T>
class basic_zstring_view
{
public:
    using string_view = std::basic_string_view<T>;
    using traits_type = typename string_view::traits_type;
    using value_type = typename string_view::value_type;
    using const_pointer = typename string_view::const_pointer;
    using const_reference = typename string_view::const_reference;
    using const_iterator = typename string_view::const_iterator;
    using const_reverse_iterator = typename string_view::const_reverse_iterator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    constexpr basic_zstring_view() noexcept
        : string_{ empty_c_string_v<value_type> }
    {}

    constexpr basic_zstring_view(const_pointer c_string) noexcept
        : string_{ c_string }
    {}

    constexpr basic_zstring_view(c_str_construct_t, const_pointer data, size_t size) noexcept
        : string_{ data, size }
    {
        D_ASSERT(is_null_terminated(data, size));
    }

    template<class C, class = std::enable_if_t<is_string_v<C> && !is_zstring_view_v<C>>>
    constexpr basic_zstring_view(const C& string) noexcept
        : basic_zstring_view{ c_str_construct, string.c_str(), std::size(string) }
    {}

    [[nodiscard]]
    constexpr operator string_view () const noexcept
    {
        return as_string_view();
    }

    [[nodiscard]]
    constexpr string_view as_string_view() const noexcept
    {
        return string_;
    }

    [[nodiscard]]
    constexpr const_pointer c_str() const noexcept
    {
        return string_.data();
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept
    {
        return string_.data();
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return string_.size();
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept
    {
        return string_.begin();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept
    {
        return string_.end();
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return string_.begin();
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return string_.end();
    }

    [[nodiscard]]
    constexpr auto operator<=>(const basic_zstring_view&) const noexcept = default;

private:
    string_view string_;
};

template<class T> [[nodiscard]]
constexpr bool operator==(const basic_zstring_view<T>& left, const basic_zstring_view<T>& right) noexcept
{
    return left.as_string_view() == right;
}

template<class T> [[nodiscard]]
constexpr bool operator!=(const basic_zstring_view<T>& left, const basic_zstring_view<T>& right) noexcept
{
    return left.as_string_view() != right;
}

using zstring_view = basic_zstring_view<char>;
using wzstring_view = basic_zstring_view<wchar_t>;
using u8zstring_view = basic_zstring_view<char8_t>;
using u16zstring_view = basic_zstring_view<char16_t>;
using u32zstring_view = basic_zstring_view<char32_t>;

[[nodiscard]]
constexpr zstring_view operator"" _zsv(const char* source, size_t length) noexcept
{
    return { c_str_construct, source, length };
}