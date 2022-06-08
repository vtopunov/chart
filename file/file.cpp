#include "file.h"

#include <bit>

#include <core/assert.h>
#include <core/underlying.h>

#include <os/os.h>

#include <file/file_io.h>

namespace file
{
    namespace
    {
        enum class access_flags : DWORD
        {
            read = GENERIC_READ,
            write = GENERIC_WRITE
        };

        [[nodiscard]]
        constexpr access_flags operator | (access_flags left, access_flags right) noexcept
        {
            return underlying_cast<access_flags>( to_underlying(left) | to_underlying(right) );
        }

        enum class share_flags : DWORD
        {
            read = FILE_SHARE_READ,
            write = FILE_SHARE_WRITE
        };

        [[nodiscard]]
        constexpr share_flags operator | (share_flags left, share_flags right) noexcept
        {
            return underlying_cast<share_flags>( to_underlying(left) | to_underlying(right) );
        }

        enum class creation_mode : DWORD
        {
            only_new = CREATE_NEW,
            new_or_trucate = CREATE_ALWAYS,
            open_existing = OPEN_EXISTING,
            new_or_open = OPEN_ALWAYS,
            trucate_existin = TRUNCATE_EXISTING
        };

        [[nodiscard]]
        constexpr file_descriptor as_file_descriptor(const HANDLE sys) noexcept
        {
            return static_cast<file_descriptor>( sys );
        }

        [[nodiscard]]
        constexpr creation_mode select_creation_mode(write_mode write_mode) noexcept
        {
            switch ( write_mode )
            {
                case write_mode::create: return creation_mode::only_new;
                case write_mode::truncate: return creation_mode::new_or_trucate;
            };

            return creation_mode::new_or_open;
        }

        [[nodiscard]]
        file_descriptor create_file(path_zstring_view path, access_flags access, creation_mode create) noexcept
        {
            constexpr auto share = share_flags::read | share_flags::write;

            return as_file_descriptor
            (
                CreateFileW
                (
                    path.c_str(),
                    to_underlying(access),
                    to_underlying(share),
                    nullptr,
                    to_underlying(create),
                    FILE_ATTRIBUTE_NORMAL,
                    nullptr
                )
            );
        }

        void set_write_mode(file_resource file, write_mode mode) noexcept
        {
            if (invalidfile != file)
            {
                if (write_mode::append == mode)
                {
                    seek(file, 0LL, seek_mode::end);
                }
            }
        }
    }

    void file_resource_deleter::operator()(file_resource file) const noexcept
    {
        if (invalidfile != file)
        {
            D_ASSERT_WITH_SIDE_EFFECTS(CloseHandle(file.fd));
        }
    }

    ro_file_resource in() noexcept
    {
        return { as_file_descriptor(GetStdHandle(STD_INPUT_HANDLE)) };
    }

    wo_file_resource out() noexcept
    {
        return { as_file_descriptor(GetStdHandle(STD_OUTPUT_HANDLE)) };
    }

    wo_file_resource err() noexcept
    {
        return { as_file_descriptor(GetStdHandle(STD_ERROR_HANDLE)) };
    }

    ro_file ro_open(path_zstring_view path) noexcept
    {
        return
        {
            resource_construct,
            create_file(path, access_flags::read, creation_mode::open_existing)
        };
    }

    wo_file wo_open(path_zstring_view path, write_mode mode) noexcept
    {
        wo_file result
        {
            resource_construct,
            create_file(path, access_flags::write, select_creation_mode(mode))
        };

        set_write_mode(result, mode);

        return result;
    }

    rw_file rw_open(path_zstring_view path, write_mode mode) noexcept
    {
        constexpr auto access = access_flags::read | access_flags::write;

        rw_file result
        {
            resource_construct,
            create_file(path, access, select_creation_mode(mode))
        };

        set_write_mode(result, mode);

        return result;
    }

    uint64_t size(file_resource file) noexcept
    {
        LARGE_INTEGER result{};
        GetFileSizeEx(file.fd, &result);
        return std::bit_cast<uint64_t>(result);
    }
}