#include "png.h"

#include <debug/debug.h>

#include <image/png.h>

#include <file/file_asset.h>

namespace
{
    gl::texture2d png_texture_from_bytes(const_buffer_view image, buffer_t& temp) noexcept
    {
        const auto space = image::png_decode_to_r8g8b8a8(image, temp);

        if (D_UNLIKELY(!space)) D_ATTRIB_UNLIKELY
        {
            const auto errc = space.error_code();
            e_debug("read png: error {}: {}", to_underlying(errc), image::png_error_string(errc).c_str());
            return {};
        }

        return gl::create_texture2d(space.sizes(), gl::R8G8B8A8, std::as_const(temp).data());
    }
}

gl::texture2d png_texture_from_asset_or_file(file::path_zstring_view path, buffer_t& temp) noexcept
{
    const auto map_file = file::asset_or_file_mmap(path);
    if (D_UNLIKELY(!map_file)) D_ATTRIB_UNLIKELY
    {
        e_debug(_PATH("can't mapping file: {}"), path.c_str());
        return {};
    }

    return png_texture_from_bytes(map_file, temp);
}

