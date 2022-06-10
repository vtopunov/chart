#include "app.h"

#include <os/os.h>

namespace ui
{
    error_code_t error_code() noexcept
    {
        return ::GetLastError();
    }
}