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
                static_assert(std::is_base_of_v<file_resource, N>);
                return { file_descriptor(-1) };
            }
        };

        file_descriptor fd;

        [[nodiscard]]
        constexpr auto operator<=>(const file_resource&) const noexcept = default;
    };

    using invalidfile_t = null_t<file_resource>;
    constexpr invalidfile_t invalidfile{};

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


    ro_file_resource in() noexcept;

    wo_file_resource out() noexcept;

    wo_file_resource err() noexcept;


    struct file_resource_deleter
    {
        void operator () (file_resource file) const noexcept;
    };

    struct ro_file : unique_resource<ro_file_resource, file_resource_deleter>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_slice);
            return static_cast<file_resource>(r());
            D_WARNING_POP;
        }
    };

    struct wo_file : unique_resource<wo_file_resource, file_resource_deleter>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_slice);
            return static_cast<file_resource>(r());
            D_WARNING_POP;
        }
    };

    struct rw_file : unique_resource<rw_file_resource, file_resource_deleter>
    {
        using unique_resource::unique_resource;

        D_WARNING_PUSH
            D_WARNING_DISABLE_MSVC(W_do_not_slice)

            [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
            return static_cast<file_resource>(r());
        }

        [[nodiscard]]
        constexpr operator ro_file_resource() const noexcept
        {
            return static_cast<ro_file_resource>(r());
        }

        [[nodiscard]]
        constexpr operator wo_file_resource() const noexcept
        {
            return static_cast<wo_file_resource>(r());
        }

        D_WARNING_POP
    };

    [[nodiscard]]
    ro_file ro_open(path_zstring_view path) noexcept;

    [[nodiscard]]
    wo_file wo_open(path_zstring_view path, write_mode mode) noexcept;

    [[nodiscard]]
    rw_file rw_open(path_zstring_view path, write_mode mode) noexcept;

    [[nodiscard]]
    uint64_t size(file_resource file) noexcept;
}