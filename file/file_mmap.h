#pragma once

#include <core/buffer_view.h>

#include <file/file.h>

namespace file
{
    struct file_mmap_resource
    {
        using view_type = const_buffer_view;

        struct _private_detail
        {
            ro_file_resource file_;
            void* fmmd_;
            const void* data_;
            size_t size_;
        }
        private_detail_;

        struct null_type
        {
            [[nodiscard]]
            constexpr operator file_mmap_resource () const noexcept
            {
                return
                {
                    _private_detail
                    {
                        invalidfile,
                        nullptr,
                        nullptr,
                        0_uz
                    }
                };
            }
        };

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!data();
        }

        [[nodiscard]]
        constexpr const void* data() const noexcept
        {
            return private_detail_.data_;
        }

        [[nodiscard]]
        constexpr size_t size() const noexcept
        {
            return private_detail_.size_;
        }

        [[nodiscard]]
        constexpr const_buffer_view view() const noexcept
        {
            return *this;
        }
    };

    struct file_mmap_resource_deleter
    {
        void operator () (file_mmap_resource resource) const noexcept;
    };

    using file_mmap = unique_resource<file_mmap_resource, file_mmap_resource_deleter>;

    [[nodiscard]]
    file_mmap mmap(path_string_view_t path) noexcept;
}