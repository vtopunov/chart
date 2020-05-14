#pragma once

#include <core/handle.h>
#include <core/assert.h>

#include <platform/egl/config.h>

namespace egl
{
    inline bool has_error() noexcept
    {
        return eglGetError() != EGL_SUCCESS;
    }

    template<class T>
    inline bool has_error(T result) noexcept
    {
        return !result || has_error();
    }

    namespace window_context
    {
        struct window_context_view
        {
            EGLDisplay display;
            EGLSurface surface;
            EGLContext context;
        };

        constexpr bool valid(window_context_view window) noexcept
        {
            return window.display && window.surface && window.context;
        }

        void close(window_context_view window) noexcept;

        bool swap_buffers(window_context_view window) noexcept;

        using safe_window = unique_handle<window_context_view>;

        safe_window create_context(EGLNativeWindowType window_handle) noexcept;
    }

    using safe_window_context = window_context::safe_window;
}