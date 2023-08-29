#include <file/file_asset.h>

#include <android/asset_manager.h>

#include <core/clamp_cast.h>

#include <common/asset_manager.h>


namespace file
{
    namespace asset
    {

        namespace
        {
            os::asset_handle_t asset_open(AAssetManager* am, path_zstring_view path) noexcept
            {
                return (am) ? AAssetManager_open(am, path.c_str(), AASSET_MODE_BUFFER) : nullptr;
            }

            struct asset_deleter
            {
                void operator()(os::asset_handle_t asset) const noexcept
                {
                    if (asset)
                    {
                        AAsset_close(asset);
                    }
                }
            };

            using asset_t = unique_resource<os::asset_handle_t, asset_deleter>;

            asset_t asset_open(path_zstring_view path) noexcept
            {
                return
                {
                    resource_construct,
                    asset_open(common::asset_manager(), path)
                };
            }

            const void* data(os::asset_handle_t asset) noexcept
            {
                return AAsset_getBuffer(asset);
            }

            size_t size(os::asset_handle_t asset) noexcept
            {
                return numeric_cast<size_t>(clamp_to_unsigned(AAsset_getLength(asset)));
            }
        }

        void asset_mmap_resource_deleter::operator()(const asset_mmap_resource& asset) const noexcept
        {
            constexpr asset_deleter close{};
            close(asset.asset_);
        }

        asset_mmap mmap(path_zstring_view path) noexcept
        {
            if (auto asset = asset_open(path)) [[likely]]
            {
                if (const auto a_data = data(asset)) [[likely]]
                {
                    if (const auto a_size = size(asset)) [[likely]]
                    {
                        return
                        {
                            resource_construct,
                            asset.release(),
                            a_data,
                            a_size
                        };
                    }
                }
            }

            return {};
        }
    }
}
