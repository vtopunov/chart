#pragma once

#include <cstdint>

#include <core/warnings.h>
#include <core/type_traits.h>

#include <os/os_detection.h>
#include <os/undef.h>


#if defined(D_OS_WINDOWS)
#define D_ONLY_OS_WINDOWS(A) A
#else
#define D_ONLY_OS_WINDOWS(A)
#endif

#if defined(D_OS_WINDOWS)
#define D_CONDITIONAL_OS_WINDOWS(A, B) A
#else
#define D_CONDITIONAL_OS_WINDOWS(A, B) B
#endif

#if defined(D_OS_ANDROID)
#define D_ONLY_OS_ANDROID(A) A
#else
#define D_ONLY_OS_ANDROID(A)
#endif

#if defined(D_OS_ANDROID)
#define D_CONDITIONAL_OS_ANDROID(A, B) A
#else
#define D_CONDITIONAL_OS_ANDROID(A, B) B
#endif


#if defined(D_OS_WINDOWS)
struct tagMSG;

#define D_OS_APICALL __stdcall

#define D_OS_HANDLE_FWD(name, def) struct name##__; namespace os { using def = name##__*; } 
D_OS_HANDLE_FWD(HBRUSH, brush_handle_t);
D_OS_HANDLE_FWD(HINSTANCE, module_handle_t);
D_OS_HANDLE_FWD(HWND, window_handle_t);
#undef D_OS_HANDLE_FWD

namespace os
{
    using handle_t = void*;
    using gdi_object_handle_t = handle_t;
    using message_t = tagMSG;

    using dword_t = unsigned long;
    using word_t = uint16_t;

    using uint_t = unsigned int;
    using word_parameter_t = size_t;
    using long_parameter_t = ptrdiff_t;
    using long_result_t = ptrdiff_t;

    typedef long_result_t (D_OS_APICALL* wndproc_t) (window_handle_t, uint_t, word_parameter_t, long_parameter_t);
}

D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_inconsistent_annotation)
extern "C" __declspec(dllimport) int D_OS_APICALL PeekMessageW
(
    os::message_t * lpMsg,
    os::window_handle_t hWnd,
    os::uint_t wMsgFilterMin,
    os::uint_t wMsgFilterMax,
    os::uint_t wRemoveMsg
);

extern "C" __declspec(dllimport) os::long_result_t D_OS_APICALL DefWindowProcW
(
    os::window_handle_t hWnd,
    os::uint_t Msg,
    os::word_parameter_t wParam,
    os::long_parameter_t lParam
);
D_WARNING_POP

#elif defined(D_OS_ANDROID)
struct android_app;
struct ANativeWindow;
struct ASensorManager;
struct ASensorEventQueue;
struct AInputEvent;
struct android_poll_source;
struct AAsset;

extern "C" int ALooper_pollAll(int timeoutMillis, int* outFd, int* outEvents, void** outData);

#endif


namespace os
{
#if defined(D_OS_WINDOWS)
    constexpr wndproc_t def_window_proc = DefWindowProcW;

#elif defined(D_OS_ANDROID)
    using module_handle_t = android_app*;
    using window_handle_t = ANativeWindow*;
    using sensor_manager_handle_t = ASensorManager*;
    using sensor_event_queue_handle_t = ASensorEventQueue*;
    using asset_handle_t = AAsset*;

#endif

    D_ONLY_OS_WINDOWS(using const_brush_handle_t = add_const_pointer_t<brush_handle_t>);
    using const_module_handle_t = add_const_pointer_t<module_handle_t>;
    using file_descriptor_t = D_CONDITIONAL_OS_WINDOWS(handle_t, int);

    namespace private_detail_osfwd_test
    {
        template<class Handle>
        constexpr bool handle_type_is_valid_v = std::conjunction_v<std::is_pointer<Handle>, std::is_class<std::remove_pointer_t<Handle>>>;

        D_ONLY_OS_WINDOWS(static_assert(handle_type_is_valid_v<brush_handle_t>));
        static_assert(handle_type_is_valid_v<window_handle_t>);
        static_assert(handle_type_is_valid_v<module_handle_t>);
    }
}
