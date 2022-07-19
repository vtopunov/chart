#include "app.h"

#include <os/os.h>

namespace ui
{
    void quit() noexcept
    {
        ::PostQuitMessage(0);
    }

    error_code_t error_code() noexcept
    {
        return ::GetLastError();
    }
}
