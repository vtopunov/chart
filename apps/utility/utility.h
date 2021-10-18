#pragma once

#include <display/gl/texture.h>
#include <file/path.h>

using namespace display;

gl::texture2d_t png_texture(file::path_string_view_t path) noexcept;
