#include "debug.h"

#include "os/os.h"

namespace private_detail_debug
{
    void output_debug_string(const char* string) noexcept
    {
        OutputDebugStringA(string);
    }

    void output_debug_string(const wchar_t* string) noexcept
    {
        OutputDebugStringW(string);
    }
}
