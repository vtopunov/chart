#pragma once

#include <filesystem>
#include <core/zstring_view.h>

namespace file
{
    using path_char_t = std::filesystem::path::value_type;
    using path_string_view_t = basic_zstring_view<path_char_t>;

#define _PATH(x) file::path_string_view_t( L##x )
}