#pragma once

#include <file/path.h>
#include <font/font.h>

namespace font_cache
{
    struct cache_deref
    {
        void operator () (::font::face_descriptor_t face) const noexcept;
    };

    using face = unique_resource<::font::face_descriptor_t, cache_deref>;
    
    void set_directory(file::path path) noexcept;

    [[nodiscard]]
    face load_font(file::path_string_view name, px::pxside_t size) noexcept;
}
