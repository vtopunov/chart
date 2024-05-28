#pragma once

#include <ui/fwd.h>


#ifdef D_OS_WINDOWS
#define D_EGLAPI __declspec(dllimport)
#define D_EGLAPIENTRY __stdcall
#else
#define D_EGLAPI
#define D_EGLAPIENTRY
#endif

using egl_display_t = void*;
using egl_surface_t = void*;
using egl_context_t = void*;
using egl_boolean_t = unsigned int; 

extern "C" 
{
    D_EGLAPI egl_boolean_t D_EGLAPIENTRY eglSwapBuffers(egl_display_t, egl_surface_t);
}

namespace egl_ui
{
    using error_code_t = uint64_t;
    using ui::module_handle_t;
    using ui::const_module_handle_t;
    using ui::window_handle_t;

    constexpr egl_boolean_t egl_false_v{};

    [[nodiscard]]
    constexpr bool egl_to_bool(egl_boolean_t value) noexcept
    {
        return egl_false_v != value;
    }
}
