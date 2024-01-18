#include "asset_manager.h"

#include <atomic>


namespace common
{
    namespace
    {
        std::atomic<AAssetManager*> asset_manager_{ nullptr };
    }

    AAssetManager* asset_manager() noexcept
    {
        return asset_manager_;
    }
    
    asset_manager_own::asset_manager_own(AAssetManager* am) noexcept
    {
        D_ASSERT(am);
        asset_manager_ = am;
    }
    
    asset_manager_own::~asset_manager_own() noexcept
    {
        asset_manager_ = nullptr;
    }
}