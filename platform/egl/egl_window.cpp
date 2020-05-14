#include "egl_window.h"

namespace egl
{
    namespace window_context
    {
        void close(window_context_view window) noexcept
        {
            if ( window.surface )
            {
                D_ASSERT(window.display);
                const bool ok = eglDestroySurface(window.display, window.surface);
                D_ASSERT(ok); (void) ok;
            }

            if ( window.context )
            {
                D_ASSERT(window.display);
                const bool ok = eglDestroyContext(window.display, window.context);
                D_ASSERT(ok); (void) ok;
            }

            if ( window.display )
            {
                eglMakeCurrent(window.display, nullptr, nullptr, nullptr);
                const bool ok = eglTerminate(window.display);
                D_ASSERT(ok); (void) ok;
            }
        }

        bool swap_buffers(window_context_view window) noexcept
        {
            return !!eglSwapBuffers(window.display, window.surface);
        }

        safe_window create_context(EGLNativeWindowType window_handle) noexcept
        {
            safe_window result;

            result->display = eglGetDisplay(nullptr);

            if ( has_error(result->display) ) 
            {
                return {};
            }

            if ( has_error(eglInitialize(result->display, nullptr, nullptr)) )
            {
                return {};
            }

            if ( has_error(eglBindAPI(EGL_OPENGL_ES_API)) )
            {
                return {};
            }

            EGLConfig config{ nullptr };
            {
                constexpr EGLint none{ EGL_NONE };
                EGLint dummy = 0;
                const auto ok = eglChooseConfig(result->display, &none, &config, 1, &dummy);
                if ( !ok || !config || has_error() )
                {
                    return {};
                }
            }

            {
                constexpr EGLint attributes[] =
                {
                    EGL_DIRECT_COMPOSITION_ANGLE, EGL_TRUE,
                    EGL_NONE
                };

                result->surface = eglCreateWindowSurface(result->display, config, window_handle, attributes);

                if ( has_error(result->surface) )
                {
                    return {};
                }
            }

            {
                constexpr EGLint attributes[] =
                {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE
                };

                result->context = eglCreateContext(result->display, config, nullptr, attributes);

                if ( has_error(result->context) )
                {
                    return {};
                }
            }

            if ( has_error(eglMakeCurrent(result->display, result->surface, result->surface, result->context)) )
            {
                return {};
            }

            return result;
        }
    }
}