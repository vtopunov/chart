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
}
