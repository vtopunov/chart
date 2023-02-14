#pragma once

#include <ui/show_command.h>

#include <egl_ui/egl_window.h>


namespace egl_ui
{
    class egl_window_builder
    {
    public:
        constexpr egl_window_builder& background(gl::rgba_colorf_t color) noexcept
        {
            background_ = color;
            return *this;
        }

        constexpr egl_window_builder& module(os::module_handle_t module) noexcept
        {
            module_ = module;
            return *this;
        }

        constexpr egl_window_builder& command_show(ui::show_command command) noexcept
        {
            command_show_ = command;
            return *this;
        }

        [[nodiscard]]
        egl_window build() const noexcept;

    private:
        gl::rgba_colorf_t background_{ gl::colors::white_f };
        os::module_handle_t module_{ nullptr };
        ui::show_command command_show_{ ui::show_command::maximazed };
    };

    [[nodiscard]]
    inline egl_window create_egl_window(os::module_handle_t module) noexcept
    {
        return egl_window_builder{}.module(module).build();
    }
}

using egl_ui::egl_window_builder;
using egl_ui::create_egl_window;
