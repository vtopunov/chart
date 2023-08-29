#include <file/file_io.h>

#include <unistd.h>

#include <core/narrow.h>
#include <core/clamp_cast.h>

#include "private/private_file.h"


namespace file
{
    namespace
    {
        template<class T>
        constexpr size_t signed_io_result_to_size_t(T result) noexcept
        {
            static_assert(std::is_signed_v<T>);
            return narrow<size_t>(clamp_to_unsigned(result));
        }
    }

    off_t seek(file_resource file, off_t offset, seek_mode mode) noexcept
    {
        static_assert(std::is_same_v<file::off_t, ::off64_t>);

        constexpr auto to_native_seek_mode = [](seek_mode mode) noexcept
        {
            static_assert(SEEK_SET == to_underlying(seek_mode::begin));
            static_assert(SEEK_CUR == to_underlying(seek_mode::current));
            static_assert(SEEK_END == to_underlying(seek_mode::end));
            return underlying_cast<int>(mode);
        };

        return ::lseek64(file_resource_to_native(file), offset, to_native_seek_mode(mode));
    }
    
    size_t write(wo_file_resource file, const void* data, size_t size) noexcept
    {
        return signed_io_result_to_size_t(::write(file_resource_to_native(file), data, size));
    }

    size_t read(ro_file_resource file, void* data, size_t size) noexcept
    {
        return signed_io_result_to_size_t(::read(file_resource_to_native(file), data, size));
    }
}