#pragma once

#include <ui/fwd.h>


using egl_display_t = void*;
using egl_surface_t = void*;
using egl_context_t = void*;
using egl_boolean_t = unsigned int; 

extern "C" 
{
    egl_boolean_t eglSwapBuffers(egl_display_t, egl_surface_t);
}

namespace egli
{
    using error_code_t = uint64_t;
    using ui::window_handle_t;

    constexpr egl_boolean_t egl_false_v{};

    [[nodiscard]]
    constexpr bool egl_to_bool(egl_boolean_t value) noexcept
    {
        return egl_false_v != value;
    }
}
