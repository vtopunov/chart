#pragma once

#include <cstdint>

#include <core/warnings.h>

#include <os/os_detection.h>
#include <os/undef.h>


#if defined(D_OS_WINDOWS)
#define D_OS_WINDOWS_ONLY(A) A
#else
#define D_OS_WINDOWS_ONLY(A)
#endif

#if defined(D_OS_WINDOWS)
#define D_CONDITIONAL_OS_WINDOWS(A, B) A
#else
#define D_CONDITIONAL_OS_WINDOWS(A, B) B
#endif

#if defined(D_OS_ANDROID)
#define D_OS_ANDROID_ONLY(A) A
#else
#define D_OS_ANDROID_ONLY(A)
#endif

#if defined(D_OS_ANDROID)
#define D_CONDITIONAL_OS_ANDROID(A, B) A
#else
#define D_CONDITIONAL_OS_ANDROID(A, B) B
#endif


#if defined(D_OS_WINDOWS)
namespace private_detail_osfwd
{
    using UINT = unsigned int;
}

#define D_OS_HANDLE_FWD(name) struct name##__; namespace private_detail_osfwd { using name = name##__*; } 
D_OS_HANDLE_FWD(HINSTANCE);
D_OS_HANDLE_FWD(HWND);
#undef D_OS_HANDLE_FWD

struct tagMSG;

D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_inconsistent_annotation)
extern "C" __declspec(dllimport) int __stdcall PeekMessageW
(
    tagMSG * lpMsg,
    private_detail_osfwd::HWND hWnd,
    private_detail_osfwd::UINT wMsgFilterMin,
    private_detail_osfwd::UINT wMsgFilterMax,
    private_detail_osfwd::UINT wRemoveMsg
);
D_WARNING_POP

#elif defined(D_OS_ANDROID)
struct android_app;
struct ANativeWindow;
struct ASensorManager;
struct ASensorEventQueue;
struct AInputEvent;
struct android_poll_source;

extern "C" int ALooper_pollAll(int timeoutMillis, int* outFd, int* outEvents, void** outData);

#endif


namespace os
{
#if defined(D_OS_WINDOWS)
    using handle_t = void*;
    using module_handle_t = private_detail_osfwd::HINSTANCE;
    using window_handle_t = private_detail_osfwd::HWND;
    using message_t = tagMSG;

    using uint_t = private_detail_osfwd::UINT;
    using dword_t = unsigned long;
    using word_t = uint16_t;


#elif defined(D_OS_ANDROID)
    using module_handle_t = android_app*;
    using window_handle_t = ANativeWindow*;
    using sensor_manager_handle_t = ASensorManager*;
    using sensor_event_queue_handle_t = ASensorEventQueue*;

#endif

    using file_descriptor_t = D_CONDITIONAL_OS_WINDOWS(handle_t, int);
}