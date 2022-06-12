#pragma once

#include <file/file.h>

namespace file
{
    using off_t = int64_t;

    constexpr off_t invalid_offset{ -1LL };
    
    enum class seek_mode : D_CONDITIONAL_OS_WINDOWS(os::dword_t, int)
    {
        begin = 0,
        current = 1,
        end = 2
    };

    off_t seek(file_resource file, off_t offset, seek_mode mode) noexcept;

    size_t write(wo_file_resource file, const void* data, size_t size) noexcept;

    size_t read(ro_file_resource file, void* data, size_t size) noexcept;
}
