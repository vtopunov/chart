#include "file_io.h"

#include <core/narrow.h>

#include <os/os.h>

namespace file
{
    namespace
    {
        using native_seek_mode_t = DWORD;
        static_assert(is_safe_numeric_conversion_v<native_seek_mode_t, seek_mode>);

        constexpr bool test_seek_mode_defs(seek_mode mode, native_seek_mode_t def) noexcept
        {
            return to_underlying(mode) == def;
        };

        static_assert(test_seek_mode_defs(seek_mode::begin, FILE_BEGIN));
        static_assert(test_seek_mode_defs(seek_mode::current, FILE_CURRENT));
        static_assert(test_seek_mode_defs(seek_mode::end, FILE_END));

        using native_io_size_t = DWORD;
    }

    offset_t seek(file_resource file, offset_t offset, seek_mode mode) noexcept
    {
        using native_offset_t = LARGE_INTEGER;

        constexpr auto to_native_offset = [] (offset_t offset) noexcept
        {
            static_assert( sizeof(native_offset_t) == sizeof(offset_t) );
            static_assert( std::is_same_v<decltype( native_offset_t::QuadPart ), offset_t> );
            return native_offset_t{ .QuadPart = offset };
        };

        constexpr auto native_error_seek = to_native_offset(error_seek);

        native_offset_t result{ native_error_seek };

        if ( !SetFilePointerEx(file.fd, to_native_offset(offset), &result, to_underlying(mode) ) )
        {
            result = native_error_seek;
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