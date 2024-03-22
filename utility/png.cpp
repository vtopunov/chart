#include "png.h"

#include <debug/debug.h>

#include <image/png.h>

#include <file/file_asset.h>


namespace
{
    gl::texture2d png_texture_from_bytes(const_buffer_view image, buffer_t& temp) noexcept
    {
        const auto result = image::png_decode_to_r8g8b8a8(image, temp);

        if (!result) [[unlikely]]
        {
            const auto errc = result.error_code();
            e_debug("read png: error {}: {}", to_underlying(errc), image::png_error_string(errc).c_str());
            return {};
        }

        return gl::create_texture2d(result);
    }
}

gl::texture2d png_texture_from_asset(file::path_zstring_view path, buffer_t& temp) noexcept
{
    const auto map_file = file::asset::mmap(path);
    if (!map_file) [[unlikely]]
    {
        e_debug(_PATH("can't mapping file: {}"), path.c_str());
        return {};
    }

    return png_texture_from_bytes(map_file, temp);
}

gl::texture2d png_texture_from_asset(file::path_zstring_view path) noexcept
{
    buffer_t temp{};
    return png_texture_from_asset(path, temp);
}

