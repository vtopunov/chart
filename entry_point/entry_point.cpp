#include "entry_point.h"

#include <os/os.h>

extern "C" int APIENTRY WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    return app_main(instance);
}