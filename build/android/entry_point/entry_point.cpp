#include <entry_point/entry_point.h>

#include <unistd.h>

#include <common/app.h>
#include <common/asset_manager.h>
#include <debug/debug.h>

#include "android_native_app_glue.h"


namespace
{
    void set_current_directory(const char* path) noexcept
    {
        if (is_null_or_zfront(path)) [[unlikely]]
        {
            e_debug("{}: set_current_directory: path is null or empty", __FILE__);
            return;
        }

        if (chdir(path)) [[unlikely]]
        {
            e_debug("chdir: path: {}, errno: {}", path, errno);
            return;
        }
    }
}

extern "C" int android_main(android_app* app)
{
    int exit_code = EXIT_SUCCESS;
    {
        const common::app_own app_own{ app };
        {
            const common::asset_manager_own asset_manager_own{ app->activity->assetManager };

            set_current_directory(app->activity->internalDataPath);

            exit_code = main();
        }
    }

    return exit_code;
}