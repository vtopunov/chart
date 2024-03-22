#include "file.h"

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
            return e_bit_or(left, right);
        }

        enum class share_flags : DWORD
        {
            read = FILE_SHARE_READ,
            write = FILE_SHARE_WRITE
        };

        [[nodiscard]]
        constexpr share_flags operator | (share_flags left, share_flags right) noexcept
        {
            return e_bit_or(left, right);
        }

        enum class creation_mode : DWORD
        {
            new_or_trucate = CREATE_ALWAYS,
            open_existing = OPEN_EXISTING,
            new_or_open = OPEN_ALWAYS
        };

        [[nodiscard]]
        constexpr file_resource_descriptor_t as_resource_descriptor(os::file_descriptor_t sys) noexcept
        {
            return static_cast<file_resource_descriptor_t>( sys );
        }

        [[nodiscard]]
        constexpr creation_mode select_creation_mode(w_open_mode mode) noexcept
        {
            return (w_open_mode::truncate == mode) ? creation_mode::new_or_trucate : creation_mode::new_or_open;
        }

        [[nodiscard]]
        file_resource_descriptor_t create_file(path_zstring_view path, access_flags access, creation_mode create) noexcept
        {
            constexpr auto share = share_flags::read | share_flags::write;

            return as_resource_descriptor
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

        void seekend_if_need(file_resource file, w_open_mode mode) noexcept
        {
            if ((w_open_mode::append == mode) && (invalidfile != file))
            {
                seek(file, 0LL, seek_mode::end);
            }
        }

        [[nodiscard]]
        file_resource_descriptor_t std_handle(DWORD no) noexcept
        {
            return as_resource_descriptor(GetStdHandle(no));
        }
    }

    void file_resource_deleter::operator()(file_resource file) const noexcept
    {
        if (invalidfile != file)
        {
            D_ASSERT_OR_UNUSED(CloseHandle(file.fd));
        }
    }

    stdin_file_resource::operator ro_file_resource() const noexcept
    {
        return { std_handle(STD_INPUT_HANDLE) };
    }

    stdout_file_resource::operator wo_file_resource() const noexcept
    {
        return { std_handle(STD_OUTPUT_HANDLE) };
    }

    stderr_file_resource::operator wo_file_resource() const noexcept
    {
        return { std_handle(STD_ERROR_HANDLE) };
    }

    ro_file ro_open(path_zstring_view path) noexcept
    {
        return
        {
            resource_construct,
            create_file(path, access_flags::read, creation_mode::open_existing)
        };
    }

    wo_file wo_open(path_zstring_view path, w_open_mode mode) noexcept
    {
        wo_file result
        {
            resource_construct,
            create_file(path, access_flags::write, select_creation_mode(mode))
        };

        seekend_if_need(result, mode);

        return result;
    }

    rw_file rw_open(path_zstring_view path, w_open_mode mode) noexcept
    {
        constexpr auto access = access_flags::read | access_flags::write;

        rw_file result
        {
            resource_construct,
            create_file(path, access, select_creation_mode(mode))
        };

        seekend_if_need(result, mode);

        return result;
    }

    uint64_t size(file_resource file) noexcept
    {
        LARGE_INTEGER result{};
        const auto is_success = !!GetFileSizeEx(file.fd, &result);
        return is_success ? narrow<uint64_t>(result.QuadPart) : 0ull;
    }
}