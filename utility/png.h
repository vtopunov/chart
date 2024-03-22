#pragma once

#include <gl/texture.h>
#include <file/path.h>


[[nodiscard]]
gl::texture2d png_texture_from_asset(file::path_zstring_view path, buffer_t& temp) noexcept;

[[nodiscard]]
gl::texture2d png_texture_from_asset(file::path_zstring_view path) noexcept;
