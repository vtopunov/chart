#pragma once

#include <ui/event_loop.h>

#if defined(D_OS_WINDOWS)
#include <ui/window.h>
#endif

#include <egl_ui/egl_instance.h>
#include <egl_ui/painting_owner.h>


namespace egl_ui
{
    namespace private_detail_run
    {
#if defined(D_OS_WINDOWS)
        inline os::window_handle_t prepare(const egl_resources& egl) noexcept
        {
            ui::show(egl.ui.app_wnd, ui::show_command::show_maximazed);
            return render_window(egl);
        }

#elif defined(D_OS_ANDROID)
        constexpr os::module_handle_t prepare(const egl_resources& egl) noexcept
        {
            return app(egl);
        }
#endif
    }

    template<class T>
    int run(const egl_resources& egl, T&& processor) noexcept
    {
        return ui::run_event_loop(private_detail_run::prepare(egl), std::forward<T>(processor));
    }

    inline int run(const egl_resources& egl) noexcept
    {
        return ui::run_event_loop(private_detail_run::prepare(egl));
    };
}
