#pragma once

#include <format>

#include <core/os.h>


inline void output_debug_string(const char* string) noexcept
{
    OutputDebugStringA(string);
}

inline void output_debug_string(const wchar_t* string) noexcept
{
    OutputDebugStringW(string);
}

template<class CharT, class... T>
void output_debug_string(const CharT* format_string, const T&... args) noexcept
{
    CharT out[256]{};
    std::format_to_n(std::data(out), std::size(out) - 1u, std::basic_string_view<CharT>{ format_string }, args...);
    output_debug_string(std::data(out));
}