#pragma once

#include <os/os_detection.h>

#if defined(D_OS_WINDOWS)


#define D_OS_WINDOWS_FWD_HANDLE(name) struct name##__; namespace private_detail_os_fwd_handle { using name = name##__*; }
D_OS_WINDOWS_FWD_HANDLE(HINSTANCE);
D_OS_WINDOWS_FWD_HANDLE(HWND);
#undef D_OS_WINDOWS_FWD_HANDLE


#elif defined(D_OS_ANDROID)

struct android_app;

#endif



namespace os
{
#if defined(D_OS_WINDOWS)

    using module_handle_t = private_detail_os_fwd_handle::HINSTANCE;
    using window_handle_t = private_detail_os_fwd_handle::HWND;

#elif defined(D_OS_ANDROID)

    using module_handle_t = android_app*;

#endif
}
