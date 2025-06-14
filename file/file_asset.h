#pragma once

#include <core/resource.h>

#include <file/path.h>


namespace file
{
    namespace asset
    {
        struct asset_mmap_resource
        {
#if defined(D_OS_ANDROID)
            os::asset_handle_t asset_;
#endif

            const void* data_;
            size_t size_;

            struct null_type
            {
                [[nodiscard]]
                constexpr operator asset_mmap_resource () const noexcept
                {
                    return
                    {
#if defined(D_OS_ANDROID)
                        nullptr,
#endif
                        nullptr,
                        0u
                    };
                }
            };

            [[nodiscard]]
            constexpr explicit operator bool() const noexcept
            {
                return !!size_;
            }

            [[nodiscard]]
            constexpr const void* data() const noexcept
            {
                return data_;
            }

            [[nodiscard]]
            constexpr size_t size() const noexcept
            {
                return size_;
            }
        };


        static_assert(std::is_same_v<decl_null_type_t<asset_mmap_resource>, asset_mmap_resource::null_type>);
        static_assert(std::is_same_v<view_t<asset_mmap_resource>, const const_byte_buffer_view>);

#if defined(D_OS_ANDROID)
        struct asset_mmap_resource_deleter
        {
            void operator () (const asset_mmap_resource& asset_or_file) const noexcept;
        };

        using asset_mmap = unique_resource<asset_mmap_resource, asset_mmap_resource_deleter>;

#else
        using asset_mmap = asset_mmap_resource;

#endif

        [[nodiscard]]
        asset_mmap mmap(path_zstring_view path) noexcept;
    }

    using asset::asset_mmap_resource;
    using asset::asset_mmap;
}



