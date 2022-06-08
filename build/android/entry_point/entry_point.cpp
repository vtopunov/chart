#include <entry_point/entry_point.h>

#include <debug/debug.h>

#include "android_native_app_glue.h"


int android_main(android_app* app)
{
    static_assert(std::is_same_v<os::module_handle_t, android_app*>);

    const auto exit_status = app_main(app);

    if (exit_status)
    {
        e_debug("EXIT FAILURE status: {}, errno: {}", exit_status, errno);
    }
    else
    {
        debug("exit success");
    }

    return exit_status;
}