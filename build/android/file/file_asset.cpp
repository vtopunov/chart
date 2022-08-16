#include <file/file_asset.h>

#include <android/asset_manager.h>

#include <core/clamp_cast.h>

#include <common/asset_manager.h>


namespace file
{
    namespace
    {
        os::asset_handle_t asset_open(AAssetManager* am, path_zstring_view path, asset_mode mode) noexcept
        {
            return (am) ? AAssetManager_open(am, path.c_str(), to_underlying(mode)) : nullptr;
        }
    }

    void asset_deleter::operator()(os::asset_handle_t asset) const noexcept
    {
        if (asset)
        {
            AAsset_close(asset);
        }
    }

    asset_t asset_open(path_zstring_view path, asset_mode mode) noexcept
    {
        return 
        {
            resource_construct,
            asset_open(common::asset_manager(), path, mode)
        };
    }

    const void* data(os::asset_handle_t asset) noexcept
    {
        return AAsset_getBuffer(asset);
    }
    
    size_t size(os::asset_handle_t asset) noexcept
    {
        return safe_numeric_cast<size_t>(clamp_to_unsigned(AAsset_getLength(asset)));
    }

    void asset_or_file_mmap_resource_deleter::operator()(const asset_or_file_mmap_resource& asset_or_file) const noexcept
    {
        const auto p = std::addressof(asset_or_file.private_detail_);
        if (const auto passet = std::get_if<asset_or_file_mmap_resource::_private_detail_asset_resource>(p))
        {
            constexpr asset_deleter close{};
            close(passet->asset_);
        }
        else if(const auto pmmap = std::get_if<file_mmap_resource>(p))
        {
            constexpr file_mmap_resource_deleter close{};
            close(*pmmap);
        }
    }

    asset_or_file_mmap_t asset_or_file_mmap(path_zstring_view path) noexcept
    {
        asset_or_file_mmap_t result;

        auto& p = as_mutable(result.r().private_detail_);
        
        if (auto asset = asset_open(path, asset_mode::buffer))
        {
            const auto data = file::data(asset);
            const auto size = file::size(asset);

            if (data && size)
            {
                p = asset_or_file_mmap_resource::_private_detail_asset_resource
                { 
                    .asset_{ asset.release() },
                    .data_{ data },
                    .size_{ size }
                };
            }
        }
        else
        {
            if (auto file = mmap(path))
            {
                p = file.release();
            }
        }

        return result;
    }
}
