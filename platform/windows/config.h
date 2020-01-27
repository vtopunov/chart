#pragma once

#include <core/assert.h>

namespace os_windows
{
#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN 1
#endif


#pragma warning(push, 0)
#include <windows.h>
#include <windowsx.h>
#pragma warning(pop)

#undef OpenFile
#undef LoadBitmap
#undef MessageBox
#undef GetObject
#undef CreateFile
#undef CreateEvent
#undef CreateDirectory
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef PathFileExists
#undef CreateFileMapping
#undef FindFirstFile
#undef FindNextFile
#undef GetTempPath
#undef GetCurrentTime
#undef SendMessage
#undef GetMessage
#undef OutputDebugString

#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
}

namespace os_windows
{
    template<class... T>
    void output_debug_string(const char* fromat, T&&... args) noexcept
    {
        char outbuf[256]{};
        const auto success = sprintf_s(outbuf, fromat, std::forward<T>(args)...) > 0;
        D_ASSERT(success);
        OutputDebugStringA(outbuf);

        if (!success)
        {
            OutputDebugStringA("\nError string formatting: ");
            OutputDebugStringA(fromat);
            OutputDebugStringA("\n");
        }
    }
}

