#pragma once

#include <core/buffer_view.h>

#include <file/file.h>


namespace file
{
    struct file_mmap_resource
    {
        struct _private_detail
        {
#ifdef D_OS_WINDOWS
            ro_file_resource file_;
            void* fmmd_;
            const void* data_;
            size_t size_;
#else
            const void* data_;
            size_t size_;
            ro_file_resource file_;
#endif
        }
        private_detail_;

        struct null_type
        {
            [[nodiscard]]
            constexpr operator file_mmap_resource () const noexcept
            {
                return
                {
#ifdef D_OS_WINDOWS
                    _private_detail
                    {
                        invalidfile,
                        nullptr,
                        nullptr,
                        0_uz
                    }
#else
                    _private_detail
                    {
                        nullptr,
                        0_uz,
                        invalidfile
                    }
#endif
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
    };

    static_assert(std::is_same_v<decl_null_type_t<file_mmap_resource>, file_mmap_resource::null_type>);
    static_assert(std::is_same_v<view_t<file_mmap_resource>, const const_buffer_view>);

    struct file_mmap_resource_deleter
    {
        void operator () (file_mmap_resource resource) const noexcept;
    };

    using file_mmap = unique_resource<file_mmap_resource, file_mmap_resource_deleter>;

    [[nodiscard]]
    file_mmap mmap(path_zstring_view path) noexcept;
}