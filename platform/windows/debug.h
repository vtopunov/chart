#pragma once

#undef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY

#pragma warning(push, 0)
#include <fmt/format.h>
#pragma warning(pop)

#include <platform/windows/config.h>

namespace os_windows
{
    template<class... T>
    void output_debug_string(const char* format_string, const T&... args) noexcept
    {
        char out[256]{};
        fmt::format_to_n(out, std::size(out)-1u, format_string, args...);
        OutputDebugStringA(out);
    }
}
