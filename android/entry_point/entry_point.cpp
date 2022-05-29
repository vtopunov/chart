#include "entry_point.h"

#include <type_traits>

#ifdef D_OS_ANDROID
#include <debug/debug.h>
#endif

#include <os/main.h>


static_assert(std::is_class_v<std::remove_pointer_t<os::module_handle_t>>);

#if defined(D_OS_WINDOWS)

extern "C" int APIENTRY WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    static_assert(std::is_same_v<os::module_handle_t, HINSTANCE>);

    return app_main(instance);
}

#elif defined(D_OS_ANDROID)

void android_main(android_app* app)
{
    static_assert(std::is_same_v<os::module_handle_t, android_app*>);

    if (const auto return_code = app_main(app))
    {
        e_debug("return code: {}", return_code);
    }
}

#endif