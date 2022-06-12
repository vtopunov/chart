#include <file/file.h>

#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include <core/narrow.h>

#include "private/file.h"

namespace file
{
    namespace
    {
        static_assert(STDIN_FILENO == to_underlying(stdin_fd.fd));
        static_assert(STDOUT_FILENO == to_underlying(stdout_fd.fd));
        static_assert(STDERR_FILENO == to_underlying(stderr_fd.fd));

        enum class open_flags
        {
            ro = O_RDONLY,
            wo = O_WRONLY,
            rw = O_RDWR,
            create = O_CREAT,
            truncate = O_TRUNC,
            append = O_APPEND
        };

        [[nodiscard]]
        constexpr open_flags operator | (open_flags left, open_flags right) noexcept
        {
            return e_or(left, right);
        }

        template<class... Args>
        file_resource_descriptor_t _open(path_zstring_view path, Args... args) noexcept
        {
            return underlying_cast<file_resource_descriptor_t>(::open(path.c_str(), underlying_cast<int>(args)...));
        }

        file_resource_descriptor_t _ro_open(path_zstring_view path) noexcept
        {
            return _open(path, open_flags::ro);
        }

        file_resource_descriptor_t _w_open(path_zstring_view path, open_flags flags) noexcept
        {
            constexpr int permissions{ S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH };
            return _open(path, open_flags::create | flags, permissions);
        }

        constexpr open_flags mode_to_flags(w_open_mode mode) noexcept
        {
            switch (mode)
            {
            case w_open_mode::truncate:
                return open_flags::truncate;
            case w_open_mode::append:
                return open_flags::append;
            default:
                break;
            }

            return {};
        }
    }

    void file_resource_deleter::operator()(file_resource file) const noexcept
    {
        if (invalidfile != file)
        {
            ::close(to_underlying(file.fd));
        }
    }

    ro_file ro_open(path_zstring_view path) noexcept
    {
        return
        {
            resource_construct,
            _ro_open(path)
        };
    }
    
    wo_file wo_open(path_zstring_view path, w_open_mode mode) noexcept
    {
        return
        {
            resource_construct,
            _w_open(path, open_flags::wo | mode_to_flags(mode))
        };
    }

    rw_file rw_open(path_zstring_view path, w_open_mode mode) noexcept
    {
        return
        {
            resource_construct,
            _w_open(path, open_flags::rw | mode_to_flags(mode))
        };
    }
    
    uint64_t size(file_resource file) noexcept
    {
        struct stat data { .st_size{} };
        const auto is_success = !::fstat(file_resource_to_native(file), &data);
        return  is_success ? narrow_cast<uint64_t>(data.st_size) : 0ull;;
    }
    

}