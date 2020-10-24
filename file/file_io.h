#pragma once

#include <file/file.h>

namespace file
{
    using offset_t = int64_t;

    inline constexpr offset_t error_seek{ -1LL };
    
    enum class seek_mode
    {
        begin,
        current,
        end
    };

    offset_t seek(file_resource file, offset_t offset, seek_mode mode) noexcept;

    size_t write(wo_file_resource file, const void* data, size_t size) noexcept;

    size_t read(ro_file_resource file, void* data, size_t size) noexcept;
}
