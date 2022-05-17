#pragma once

#include <core/warnings.h>

D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_incorrect_logical_or)
D_WARNING_DISABLE_MSVC(W_redundant_code__left_and_right_subexpressions_are_identical)

#include <fmt/xchar.h>
#include <fmt/format.h>

D_WARNING_POP


#include <core/string_char.h>

namespace private_detail_debug
{
    void output_debug_string(const char* string) noexcept;

    void output_debug_string(const wchar_t* string) noexcept;

    template<class CharT, class... Args>
    void tag_debug(CharT tag, std::basic_string_view<CharT> format_string, const Args&... args) noexcept
    {
        constexpr size_t n_buf{ 512u };
        CharT buffer[n_buf];
        *buffer = tag;

        {
            constexpr CharT tagend[]{ ':', ' ' };
            constexpr CharT endl[]{ '\n', '\0' };
            std::copy(std::cbegin(tagend), std::cend(tagend), buffer + 1);

            {
                constexpr auto n_tag = std::size(tagend) + 1u;
                static_assert(n_tag < n_buf);
                constexpr auto n_max = n_buf - (n_tag + std::size(endl));

                const auto result = fmt::format_to_n
                (
                    buffer + n_tag,
                    n_max,
                    format_string,
                    args...
                );

                std::copy(std::cbegin(endl), std::cend(endl), result.out);
            }
        }

        output_debug_string(buffer);
    }
}


template<class FormatString, class... Args>
void debug(const FormatString& format_string, const Args&... args) noexcept
{
    using format_string_char_t = string_char_t<FormatString>;
    constexpr format_string_char_t tag{ 'D' };
    private_detail_debug::tag_debug<format_string_char_t>(tag, format_string, args...);
}

template<class FormatString, class... Args>
void e_debug(const FormatString& format_string, const Args&... args) noexcept
{
    using format_string_char_t = string_char_t<FormatString>;
    constexpr format_string_char_t tag{ 'E' };
    private_detail_debug::tag_debug<format_string_char_t>(tag, format_string, args...);
}