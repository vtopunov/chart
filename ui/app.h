#pragma once

#include <ui/fwd.h>

namespace ui
{
#ifdef D_OS_ANDROID
        [[nodiscard]] 
        window_handle_t app_window(const_module_handle_t app) noexcept;

        using cmd_callback_t = void (*)(os::module_handle_t, int32_t);
        using input_event_callback_t = int32_t(*)(os::module_handle_t, AInputEvent*);

        [[nodiscard]] 
        void* user_data(const_module_handle_t app) noexcept;
        void set_user_data(module_handle_t app, void* data) noexcept;
        void set_cmd_callback(module_handle_t app, cmd_callback_t callback) noexcept;
        void set_input_event_callback(module_handle_t app, input_event_callback_t callback) noexcept;

        void quit(const_module_handle_t app) noexcept;

#else
        void quit() noexcept;

        inline void quit(const_module_handle_t) noexcept
        {
            quit();
        }
#endif
 
    using error_code_t = D_CONDITIONAL_OS_WINDOWS(dword_t, int);

    [[nodiscard]]
    error_code_t error_code() noexcept;
}

using ui::quit;
