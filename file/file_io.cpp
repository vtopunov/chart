#include "file_io.h"

#include <core/narrow.h>

#include <os/os.h>

namespace file
{
    namespace
    {
        using native_io_size_t = DWORD;
    }

    off_t seek(file_resource file, off_t offset, seek_mode mode) noexcept
    {
        using native_offset_t = LARGE_INTEGER;

        constexpr auto to_native_offset = [] (off_t offset) noexcept
        {
            static_assert( sizeof(native_offset_t) == sizeof(off_t) );
            static_assert( std::is_same_v<decltype( native_offset_t::QuadPart ), off_t> );
            return native_offset_t{ .QuadPart = offset };
        };

        constexpr auto to_native_seek_mode = [](seek_mode mode) noexcept
        {
            static_assert(FILE_BEGIN == to_underlying(seek_mode::begin));
            static_assert(FILE_CURRENT == to_underlying(seek_mode::current));
            static_assert(FILE_END == to_underlying(seek_mode::end));
            return underlying_cast<DWORD>(mode);
        };

        constexpr auto native_invalid_offset = to_native_offset(invalid_offset);

        native_offset_t result{ native_invalid_offset };

        if ( !SetFilePointerEx(file.fd, to_native_offset(offset), &result, to_native_seek_mode(mode) ) )
        {
            result = native_invalid_offset;
        }

        return result.QuadPart;
    }

    size_t read(ro_file_resource file, void* data, size_t size) noexcept
    {
        native_io_size_t result{ 0u };

        if ( !ReadFile(file.fd, data, narrow_cast<native_io_size_t>(size), &result, nullptr) )
        {
            if ( result == size )
            {
                result = 0u;
            }
        }

        return result;
    }

    size_t write(wo_file_resource file, const void* data, size_t size) noexcept
    {
        native_io_size_t result{ 0u };

        if ( !WriteFile(file.fd, data, narrow_cast<native_io_size_t>( size ), &result, nullptr) )
        {
            if ( result == size )
            {
                result = 0u;
            }
        }

        return result;
    }
}