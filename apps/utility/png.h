#pragma once

#include <core/buffer.h>

#include <gl/texture.h>
#include <file/path.h>

struct png_reader
{
    byte_buffer_t buffer;

    gl::texture2d_t texture_from_file(file::path_string_view_t path) noexcept;

    gl::texture2d_t texture_from_bytes(const_buffer_view image) noexcept;
};
