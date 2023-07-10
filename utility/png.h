#pragma once

#include <core/buffer.h>

#include <gl/texture.h>
#include <file/path.h>


[[nodiscard]]
gl::texture2d png_texture_from_asset(file::path_zstring_view path, buffer_t& temp) noexcept;

[[nodiscard]]
inline gl::texture2d png_texture_from_asset(file::path_zstring_view path) noexcept
{
    buffer_t temp;
    return png_texture_from_asset(path, temp);
}
