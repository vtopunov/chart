#include <entry_point/entry_point.h>

#include <unistd.h>

#include <debug/debug.h>
#include <common/asset_manager.h>

#include "android_native_app_glue.h"


namespace
{
    void set_current_directory(const char* path) noexcept
    {
        if (D_UNLIKELY(is_null_or_empty(path)))
        {
            e_debug("{}: set_current_directory: path is null or empty", __FILE__);
            return;
        }

        if (D_UNLIKELY(chdir(path)))
        {
            e_debug("chdir: path: {}, errno: {}", path, errno);
            return;
        }
    }
}

extern "C" int android_main(android_app* app)
{
    D_ASSERT(app);
    D_ASSERT(app->activity);

    const common::asset_manager_own asset_manager_own{ app->activity->assetManager };

    set_current_directory(app->activity->internalDataPath);

    return app_main(app);
}