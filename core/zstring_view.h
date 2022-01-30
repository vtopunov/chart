#pragma once

#include <type_traits>	

template <class, class = void>
struct has_c_str : std::false_type
{};

template <class T>
struct has_c_str<T, std::void_t<decltype(std::declval<T>().c_str())>>
    : std::true_type
{};

template<class T>
class basic_zstring_view;

template <class T>
struct is_zstring_view : std::false_type
{};

template <class T>
struct is_zstring_view<basic_zstring_view<T>> : std::true_type
{};

template <class T>
inline constexpr bool is_zstring_view_compatible_v = std::conjunction_v
<
    std::negation<is_zstring_view<std::remove_cvref_t<T>>>, 
    has_c_str<T>
>;

template<class T>
class basic_zstring_view
{
public:
    using value_type = std::remove_const_t<T>;
    using const_pointer = const value_type*;

    static constexpr value_type zero{}; 

    constexpr basic_zstring_view() noexcept
        : c_string_{ &zero }
    {}

    constexpr basic_zstring_view(const basic_zstring_view&) noexcept = default;

    constexpr basic_zstring_view(const_pointer c_string) noexcept
        : c_string_{ c_string }
    {}

    template<class C, std::enable_if_t<is_zstring_view_compatible_v<C>, int> = 0>
    constexpr basic_zstring_view(const C& string) noexcept
        : basic_zstring_view{ string.c_str() }
    {}

    constexpr basic_zstring_view& operator = (const basic_zstring_view&) noexcept = default;

    template<class C, std::enable_if_t<is_zstring_view_compatible_v<C>, int> = 0>
    constexpr basic_zstring_view& operator = (const C& string) noexcept
    {
        c_string_ = string.c_str();
        return *this;
    }

    constexpr basic_zstring_view& operator = (const_pointer c_string) noexcept
    {
        c_string_ = c_string;
        return *this;
    }

    [[nodiscard]]
    constexpr const_pointer c_str() const noexcept
    {
        return c_string_;
    }

private:
    const_pointer c_string_;
};

using zstring_view = basic_zstring_view<char>;
using wzstring_view = basic_zstring_view<wchar_t>;
using u8zstring_view = basic_zstring_view<char8_t>;
using u16zstring_view = basic_zstring_view<char16_t>;
using u32zstring_view = basic_zstring_view<char32_t>;

constexpr zstring_view operator"" _zsv(const char* source, size_t) noexcept
{
    return source;
}