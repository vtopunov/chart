#pragma once

#include <compare>

#include <core/resouce.h>
#include <file/path.h>

namespace file
{
    enum class write_mode
    {
        create,
        rewrite,
        truncate,
        append
    };

    struct _file_descriptor
    {};

    using file_descriptor = _file_descriptor*;

    struct file_resource
    {
        struct null_type
        {
            template<class N>
            constexpr operator N () const noexcept
            {
                return N{ file_descriptor(-1) };
            }
        };

        file_descriptor fd;

        [[nodiscard]]
        constexpr auto operator<=>(const file_resource&) const noexcept = default;
    };

    using invalidfile_t = null_t<file_resource>;

    inline constexpr invalidfile_t invalidfile{};

    struct ro_file_resource : file_resource
    {};

    struct wo_file_resource : file_resource
    {};

    struct rw_file_resource : file_resource
    {
        [[nodiscard]]
        constexpr operator ro_file_resource() const noexcept
        {
            return ro_file_resource{ fd };
        }

        [[nodiscard]]
        constexpr operator wo_file_resource() const noexcept
        {
            return wo_file_resource{ fd };
        }
    };


    struct file_resource_deleter
    {
        void operator () (file_resource file, resource_destroy_t) const noexcept;
    };


    struct ro_file : public unique_resource<ro_file_resource, file_resource_deleter>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
#pragma warning(push)
#pragma warning(disable : 26437) //  Don't slice
            return resource();
#pragma warning(pop)
        }
    };

    struct wo_file : public unique_resource<wo_file_resource, file_resource_deleter>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
#pragma warning(push)
#pragma warning(disable : 26437) //  Don't slice
            return resource();
#pragma warning(pop)
        }
    };

    struct rw_file : public unique_resource<rw_file_resource, file_resource_deleter>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
            return resource();
        }

        [[nodiscard]]
        constexpr operator ro_file_resource() const noexcept
        {
            return resource();
        }

        [[nodiscard]]
        constexpr operator wo_file_resource() const noexcept
        {
            return resource();
        }
    };

    [[nodiscard]]
    ro_file ro_open(path_string_view_t path) noexcept;

    [[nodiscard]]
    wo_file wo_open(path_string_view_t path, write_mode mode) noexcept;

    [[nodiscard]]
    rw_file rw_open(path_string_view_t path, write_mode mode) noexcept;

    [[nodiscard]]
    uint64_t size(file_resource file) noexcept;
}