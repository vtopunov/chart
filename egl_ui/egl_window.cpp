#include "egl_window.h"

#include <egl_ui/ui_window.h>


namespace egl_ui
{
    void egl_window_resource_collector::operator()(const egl_window_resource& r) const noexcept
    {
        if (r.surface)
        {
            D_ASSERT_OR_UNUSED(eglDestroySurface(r.display, r.surface));
        }

        if (r.context)
        {
            D_ASSERT_OR_UNUSED(eglDestroyContext(r.display, r.context));
        }

        if (r.display)
        {
            eglMakeCurrent(r.display, nullptr, nullptr, nullptr);
            D_ASSERT_OR_UNUSED(eglTerminate(r.display));
        }

        constexpr ui_window_resource_collector close{};
        close(r.ui);
    }
}

