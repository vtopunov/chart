#pragma once

#include <egl/window.h>
#include <ui/event_loop.h>

namespace egl
{
    template<class T>
    int run_event_loop(const window_resources& egl, T&& processor) noexcept
    {
        return ui::run_event_loop(egl.renderer_wnd, std::forward<T>(processor));
    }

    inline int run_event_loop(const window_resources& egl) noexcept
    {
        return ui::run_event_loop(egl.renderer_wnd);
    };

    namespace private_detail_run_initialize
    {
        inline void run_initialize(const window_resources& egl) noexcept
        {
            show(egl, ui::show_command::show_maximazed);
        }
    }

    template<class T>
    int run(const window_resources& egl, T&& processor) noexcept
    {
        private_detail_run_initialize::run_initialize(egl);
        return run_event_loop(egl, std::forward<T>(processor));
    }

    inline int run(const window_resources& egl) noexcept
    {
        private_detail_run_initialize::run_initialize(egl);
        return run_event_loop(egl);
    };
}
