#pragma once

#include <filesystem>
#include <core/zstring_view.h>

namespace file
{
    using path_char_t = std::filesystem::path::value_type;
    using path_string_view_t = basic_zstring_view<path_char_t>;

    namespace literals
    {
        [[nodiscard]]
        constexpr path_string_view_t operator"" _path(const path_char_t* source, size_t length) noexcept
        {
            return { c_str_construct, source, length };
        }
    }
}

using namespace file::literals;

#define _PATH(x) L##x##_path