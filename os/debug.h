#pragma once

#include <format>

#include <core/string_char.h>

#include <os/os.h>

namespace private_detail_debug
{
    inline void output_debug_string(const char* string) noexcept
    {
        OutputDebugStringA(string);
    }

    inline void output_debug_string(const wchar_t* string) noexcept
    {
        OutputDebugStringW(string);
    }

    template<class CharT, CharT tag, class... T>
    void tag_debug(std::basic_string_view<CharT> format_string, const T&... args) noexcept
    {
        constexpr CharT stag[]{ tag, ':', ' ' };
        constexpr CharT endl[]{ '\n', '\0' };

        constexpr size_t n_buf{ 512u };
        CharT buffer[n_buf];
        memcpy(buffer, stag, sizeof(stag));

        {
            constexpr auto n_tag = std::size(stag);
            static_assert(n_tag < n_buf);
            constexpr auto n_max = n_buf - (n_tag + std::size(endl));

            const auto result = std::format_to_n
            (
                buffer + n_tag,
                n_max,
                format_string,
                args...
            );

            memcpy(result.out, endl, sizeof(endl));
        }

        output_debug_string(buffer);
    }
}


template<class String, class... T>
void debug(const String& format_string, const T&... args) noexcept
{
    using namespace private_detail_debug;
    using char_t = string_char_t<String>;
    constexpr char_t tag{ 'D' };
    tag_debug<char_t, tag>(format_string, args...);
}

template<class String, class... T>
void e_debug(const String& format_string, const T&... args) noexcept
{
    using namespace private_detail_debug;
    using char_t = string_char_t<String>;
    constexpr char_t tag{ 'E' };
    tag_debug<char_t, tag>(format_string, args...);
}