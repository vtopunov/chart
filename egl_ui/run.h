#pragma once

#include <ui/event_loop.h>

#if defined(D_OS_WINDOWS)
#include <ui/window.h>
#endif

#include <egl_ui/egl_window_builder.h>
#include <egl_ui/painting_owner.h>


namespace egl_ui
{
    namespace private_detail_run
    {
#if defined(D_OS_WINDOWS)
        constexpr os::window_handle_t input_event_source(const egl_window_resource& egl) noexcept
        {
            return render_window(egl);
        }

#elif defined(D_OS_ANDROID)
        constexpr os::module_handle_t input_event_source(const egl_window_resource& egl) noexcept
        {
            return as_mutable_pointer(egl.ui.app);
        }
#endif
    }

    template<class T>
    int run(const egl_window_resource& egl, T&& processor) noexcept
    {
#if defined(D_OS_WINDOWS)
        const auto size_event_processing = ui::create_event_processor
        (
            app_window(egl) ,
            as_mutable_pointer(std::addressof(processor)),
            ui::one_event_callback_v<ui::event_style::size, T>
        );
#endif

        return ui::run_event_loop(private_detail_run::input_event_source(egl), as_reference(processor));
    }

    inline int run(const egl_window_resource& egl) noexcept
    {
        constexpr struct {} nop{};
        return run(egl, nop);
    };
}
