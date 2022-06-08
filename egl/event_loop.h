#pragma once

#include <ui/window.h>
#include <ui/event_loop.h>

#include <egl/instance.h>

namespace egl
{
    namespace private_detail_run_initialize
    {
        inline void run_initialize(const egl_resources& egl) noexcept
        {
#if defined(D_OS_WINDOWS)
            ui::show(egl.ui.app_wnd, ui::show_command::show_maximazed);
#endif
        }
    }

    template<class T>
    int run(const egl_resources& egl, T&& processor) noexcept
    {
        private_detail_run_initialize::run_initialize(egl);
        return ui::run_event_loop(render_window(egl.ui), std::forward<T>(processor));
    }

    inline int run(const egl_resources& egl) noexcept
    {
        private_detail_run_initialize::run_initialize(egl);
        return ui::run_event_loop(render_window(egl.ui));
    };
}
