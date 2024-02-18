#pragma once

#include <core/utility.h>


class AAssetManager;

namespace common
{
    struct asset_manager_own
    {
        D_DISABLE_COPYMOVE_CA(asset_manager_own);

        explicit asset_manager_own(AAssetManager* am) noexcept;

        ~asset_manager_own() noexcept;
    };

    [[nodiscard]]
    AAssetManager* asset_manager() noexcept;
}