#include "entry_point.h"

#include <os/os.h>


extern "C" int APIENTRY WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
    return main();
}