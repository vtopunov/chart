#pragma once

#include <core/buffer.h>

#include <gl/texture.h>
#include <file/path.h>

struct png_reader
{
    byte_buffer buffer;

    gl::texture2d texture_from_file(file::path_zstring_view path) noexcept;

    gl::texture2d texture_from_bytes(const_buffer_view image) noexcept;
};
