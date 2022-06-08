#include "entry_point.h"

#include <os/os.h>

extern "C" int APIENTRY WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    static_assert(std::is_same_v<os::module_handle_t, HINSTANCE>);

    return app_main(instance);
}