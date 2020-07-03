#include "egl_window.h"

#include <algorithm>

#include <core/span.h>

namespace egl
{
    namespace window_context
    {
        namespace
        {
            bool succesed() noexcept
            {
                return eglGetError() == EGL_SUCCESS;
            }

            template<class T>
            bool succesed(const T value) noexcept
            {
                return value && succesed();
            }

            template<class T>
            bool set_and_check(T& value, const T new_value) noexcept
            {
                value = new_value;
                return succesed(new_value);
            }
        }

        void close(window_context_view window) noexcept
        {
            if (window.surface)
            {
                D_ASSERT(window.display);
                const bool ok = eglDestroySurface(window.display, window.surface);
                D_ASSERT(ok); (void) ok;
            }

            if (window.context)
            {
                D_ASSERT(window.display);
                const bool ok = eglDestroyContext(window.display, window.context);
                D_ASSERT(ok); (void) ok;
            }

            if (window.display)
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

        constexpr EGLint egl_none = EGL_NONE;

        span<EGLint> egl_add_attribute(span<EGLint> attributes, EGLint attribute, EGLint value) noexcept
        {
            if (attributes.size() > 2)
            {
                auto it = attributes.begin();
                const auto end = std::prev(attributes.cend());

                for (; it != end; ++it)
                {
                    if (*it == egl_none)
                    {
                        *it = attribute;
                        *++it = value;
                        *++it = egl_none;
                        return { it, narrow_cast<size_t>(attributes.cend() - it) };
                    }
                }
            }

            return {};
        }

        safe_window create_context(EGLNativeWindowType window_handle) noexcept
        {
            safe_window result;

            if (!window_handle)
            {
                return nullptr;
            }

            {
#pragma warning(push)
#pragma warning(disable : 26490) // Don't use reinterpret_cast
                const auto eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>( eglGetProcAddress("eglGetPlatformDisplayEXT") );
#pragma warning(pop)

                if (!eglGetPlatformDisplayEXT)
                {
                    return nullptr;
                }

                constexpr EGLint display_attributes[] =
                {
                    EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE,
                    EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE,
                    EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE,
                    egl_none
                };

                if (!set_and_check(result->display, eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, nullptr, display_attributes)))
                {
                    return nullptr;
                }
            }

            if (!succesed(eglInitialize(result->display, nullptr, nullptr)))
            {
                return nullptr;
            }

            const struct
            {
                const char* extensions_string;

                bool has(const char* extention_string) const
                {
                    return !!strstr(extensions_string, extention_string);
                }
            }
            display_extensions
            {
                eglQueryString(result->display, EGL_EXTENSIONS)
            };

            if (!succesed(display_extensions.extensions_string))
            {
                return nullptr;
            }

            if (!succesed(eglBindAPI(EGL_OPENGL_ES_API)))
            {
                return nullptr;
            }

            EGLConfig config{nullptr};
            {
                constexpr EGLint config_attributes[] =
                {
                    EGL_RED_SIZE,       8,
                    EGL_GREEN_SIZE,     8,
                    EGL_BLUE_SIZE,      8,
                    EGL_ALPHA_SIZE,     8,
                    EGL_DEPTH_SIZE,     24,
                    EGL_STENCIL_SIZE,   8,
                    egl_none
                };

                EGLint config_count{0};
                const auto choose_ok = eglChooseConfig(result->display, config_attributes, &config, 1, &config_count);
                if (!choose_ok || !config || ( config_count != 1 ) || !succesed() )
                {
                    return nullptr;
                }
            }

            {
                EGLint surface_attributes[] =
                {
                    EGL_DIRECT_COMPOSITION_ANGLE, EGL_TRUE,
                    egl_none, egl_none,
                    egl_none
                };

                
                if (display_extensions.has("EGL_NV_post_sub_buffer"))
                {
                    span<EGLint> attributes{surface_attributes};
                    egl_add_attribute(attributes, EGL_POST_SUB_BUFFER_SUPPORTED_NV, EGL_TRUE);
                    D_ASSERT(attributes.data());
                }
                
                if (!set_and_check(result->surface, eglCreateWindowSurface(result->display, config, window_handle, surface_attributes)))
                {
                    return nullptr;
                }
            }

            {
                EGLint context_attributes[] =
                {
                    EGL_CONTEXT_OPENGL_DEBUG, EGL_FALSE,
                    EGL_CONTEXT_OPENGL_NO_ERROR_KHR, EGL_FALSE,
                    egl_none, egl_none,
                    egl_none, egl_none,
                    egl_none
                };

                if (display_extensions.has("EGL_KHR_create_context"))
                {
                    span<EGLint> attributes{ context_attributes };

                    attributes = egl_add_attribute(attributes, EGL_CONTEXT_MAJOR_VERSION_KHR, 3);
                    attributes = egl_add_attribute(attributes, EGL_CONTEXT_MINOR_VERSION_KHR, 1);

                    D_ASSERT(attributes.data());
                }

                if (!set_and_check(result->context, eglCreateContext(result->display, config, nullptr, context_attributes)))
                {
                    return nullptr;
                }
            }

            if (!succesed(eglMakeCurrent(result->display, result->surface, result->surface, result->context)))
            {
                return nullptr;
            }

            return result;
        }
    }
}