#pragma once

#include <variant>

#if defined(D_OS_ANDROID)
#include <os/fwd.h>
#endif

#include <file/file_mmap.h>


namespace file
{
#if defined(D_OS_ANDROID)
    struct asset_deleter
    {
        void operator () (os::asset_handle_t asset) const noexcept;
    };

    enum class asset_mode
    {
        unknown,
        random,
        streaming,
        buffer
    };

    using asset_t = unique_resource<os::asset_handle_t, asset_deleter>;

    asset_t asset_open(path_zstring_view path, asset_mode mode) noexcept;

    const void* data(os::asset_handle_t asset) noexcept;

    size_t size(os::asset_handle_t asset) noexcept;
#endif

    struct asset_or_file_mmap_resource
    {
        using view_type = const_buffer_view;

        struct _private_detail_asset_resource
        {
#if defined(D_OS_ANDROID)
            os::asset_handle_t asset_;
#endif

            const void* data_;
            size_t size_;

            constexpr operator const_buffer_view() const noexcept
            {
                return { data_, size_ };
            }
        };

        std::variant<std::monostate, _private_detail_asset_resource, file_mmap_resource> private_detail_;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return 0 != private_detail_.index();
        }

        [[nodiscard]]
        constexpr operator const_buffer_view() const noexcept
        {
            const auto p = std::addressof(private_detail_);

            if (const auto asset = std::get_if<_private_detail_asset_resource>(p))
            {
                return *asset;
            }

            if (const auto mmap = std::get_if<file_mmap_resource>(p))
            {
                return *mmap;
            }

            return {};
        }
    };

    struct asset_or_file_mmap_resource_deleter
    {
        void operator () (const asset_or_file_mmap_resource& asset_or_file) const noexcept;
    };

    using asset_or_file_mmap_t = unique_resource<asset_or_file_mmap_resource, asset_or_file_mmap_resource_deleter>;

    [[nodiscard]]
    asset_or_file_mmap_t asset_or_file_mmap(path_zstring_view path) noexcept;
}



