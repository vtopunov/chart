#pragma once

#include <ui/event_loop.h>

#if defined(D_OS_WINDOWS)
#include <ui/window.h>
#endif

#include <egl_ui/egl_window.h>
#include <egl_ui/painting_owner.h>


namespace egl_ui
{
    namespace private_detail_run
    {
#if defined(D_OS_WINDOWS)
        inline os::window_handle_t prepare(const egl_window_resource& egl) noexcept
        {
            ui::show(egl.ui.app_wnd, ui::show_command::show_maximazed);
            return render_window(egl);
        }

#elif defined(D_OS_ANDROID)
        constexpr os::module_handle_t prepare(const egl_window_resource& egl) noexcept
        {
            return as_mutable_pointer(egl.ui.app);
        }
#endif
    }

    template<class T>
    int run(const egl_window_resource& egl, T&& processor) noexcept
    {
        return ui::run_event_loop(private_detail_run::prepare(egl), std::forward<T>(processor));
    }

    inline int run(const egl_window_resource& egl) noexcept
    {
        return ui::run_event_loop(private_detail_run::prepare(egl));
    };
}
