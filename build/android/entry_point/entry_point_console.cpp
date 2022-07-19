#include "entry_point_console.h"

#include <os/fwd.h>

int app_main(os::module_handle_t)
{
    return main();
}
