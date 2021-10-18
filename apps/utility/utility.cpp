#include "utility.h"

#include <core/uninitialized_dynarray.h>
#include <core/debug.h>

#include <image/png.h>

#include <file/file_mmap.h>

gl::texture2d_t png_texture(file::path_string_view_t path) noexcept
{
    constexpr auto png_format = image::png_format::RGBA8;
    constexpr auto gl_format = gl::R8G8B8A8;

    const auto map_file = file::mmap(path);
    if (!map_file)
    {
        output_debug_string(L"can't mapping file: {}\n", path.as_string_view());
        return {};
    }

    const auto png = image::png_instance();

    const auto accept_png_errno = [] (image::png_errno errc) noexcept
    {
        const auto is_error = errc != image::png_errno::OK;

        if (is_error)
        {
            output_debug_string("png error: {}: {}\n", to_underlying(errc), image::png_error_string(errc).c_str());
        }

        return is_error;
    };

    if (accept_png_errno(image::png_set_buffer(png, map_file)))
    {
        return {};
    }

    const image::png_header png_header{ png };

    size_t size = 0;
    if (accept_png_errno(image::png_decoded_image_size(png, png_format, &size)))
    {
        return {};
    }

    if (!size)
    {
        output_debug_string(L"empty png image: {}\n", path.as_string_view());
        return {};
    }

    uninitialized_dynarray<std::byte> buffer{ size };

    if (!buffer)
    {
        output_debug_string("out of memory: size = {}\n", size);
        return {};
    }

    if (accept_png_errno(image::png_decode_image(png, png_format, buffer)))
    {
        return {};
    }

    return gl::create_texture2d(png_header.width(), png_header.height(), gl_format, buffer.data());
}