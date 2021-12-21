#include "png.h"

#include <core/debug.h>

#include <image/png.h>

#include <file/file_mmap.h>

gl::texture2d_t png_reader::texture_from_file(file::path_string_view_t path) noexcept
{
    const auto map_file = file::mmap(path);
    if (!map_file)
    {
        output_debug_string(L"can't mapping file: {}\n", path.c_str());
        return {};
    }

    return texture_from_bytes(map_file);
}

gl::texture2d_t png_reader::texture_from_bytes(const_buffer_view image) noexcept
{
    gl::texture2d_t result;
    
    {
        image::rgba32_pixmap_t pixmap;

        {
            [[maybe_unused]]
            const temp_swap swap{ pixmap, buffer };

            const auto errc = image::png_decode_image(image, pixmap);

            if (errc != image::png_errno::OK)
            {
                output_debug_string("read png: error {}: {}\n", to_underlying(errc), image::png_error_string(errc).c_str());
                return {};
            }

            result = gl::create_texture2d(pixmap);
        }
    }

    return result;
}
