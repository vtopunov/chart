#include "png.h"

#include <core/temp_swap.h>

#include <os/debug.h>

#include <image/png.h>

#include <file/file_mmap.h>

gl::texture2d png_reader::texture_from_file(file::path_zstring_view path) noexcept
{
    const auto map_file = file::mmap(path);
    if (!map_file)
    {
        e_debug(L"can't mapping file: {}", path.c_str());
        return {};
    }

    return texture_from_bytes(map_file);
}

gl::texture2d png_reader::texture_from_bytes(const_buffer_view image) noexcept
{
    gl::texture2d result;
    
    {
        image::pixrgba32map pixmap;

        {
            [[maybe_unused]]
            const temp_swap swap{ pixmap, buffer };

            const auto errc = image::png_decode_image(image, pixmap);

            if (errc != image::png_errno::OK)
            {
                e_debug("read png: error {}: {}", to_underlying(errc), image::png_error_string(errc).c_str());
                return {};
            }

            result = gl::create_texture2d(pixmap);
        }
    }

    return result;
}
