#include "file_asset.h"

#include <os/os.h>


namespace file
{
    namespace asset
    {
        namespace
        {
            [[nodiscard]]
            constexpr LPCWSTR rt_rcdata_w() noexcept
            {
#pragma push_macro("MAKEINTRESOURCE")
#undef MAKEINTRESOURCE
#define MAKEINTRESOURCE MAKEINTRESOURCEW
                return RT_RCDATA;
#pragma pop_macro("MAKEINTRESOURCE")
            }

            struct asset_resource
            {
                os::module_handle_t module;
                HRSRC resource_info;

                constexpr explicit operator bool() const noexcept
                {
                    return resource_info;
                }

                [[nodiscard]]
                const void* data() const noexcept
                {
                    if (const auto resource = LoadResource(module, resource_info)) [[likely]]
                    {
                        return LockResource(resource);
                    }

                    return nullptr;
                }

                [[nodiscard]]
                size_t size() const noexcept
                {
                    return safe_numeric_cast<size_t>(SizeofResource(module, resource_info));
                }
            };

            [[nodiscard]]
            asset_resource find_asset(path_zstring_view path) noexcept
            {
                const auto module = GetModuleHandleW(nullptr);

                return
                {
                    .module{ module },
                    .resource_info{ FindResourceW(module, path.c_str(), rt_rcdata_w()) }
                };
            }
        }

        asset_mmap mmap(path_zstring_view path) noexcept
        {
            if (auto asset = find_asset(path)) [[likely]]
            {
                if (const auto data = asset.data()) [[likely]]
                {
                    if (const auto size = asset.size()) [[likely]]
                    {
                        return { data, size };
                    }
                }
            }

            return {};
        }
    }
}