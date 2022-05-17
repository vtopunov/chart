#pragma once

#include <core/buffer.h>

#include <gl/texture.h>
#include <file/path.h>

gl::texture2d png_texture_from_file(file::path_zstring_view path, buffer_t& temp) noexcept;

gl::texture2d png_texture_from_bytes(const_buffer_view image, buffer_t& temp) noexcept;


inline gl::texture2d png_texture_from_file(file::path_zstring_view path) noexcept
{
    buffer_t temp;
    return png_texture_from_file(path, temp);
}

inline gl::texture2d png_texture_from_bytes(const_buffer_view image) noexcept
{
    buffer_t temp;
    return png_texture_from_bytes(image, temp);
}