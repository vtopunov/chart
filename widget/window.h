#pragma once

#include <core/buffer_view.h>

#include <egl_ui/egl_window.h>

#include <widget/window_fwd.h>
#include <widget/shaders.h>


namespace widget
{
    struct window : public egl_window
    {
        shaders shaders{};
        buffer_t temp_buffer{};
        pxsize2d user_sizes_cache{};

        constexpr window(egl_window&& egl) noexcept
            : egl_window{ std::move(egl) }
        {}

        D_DISABLE_COPY_MOVE(window);

        constexpr operator buffer_view () const noexcept
        {
            return temp_buffer_view();
        }

        [[nodiscard]]
        constexpr buffer_view temp_buffer_view() const noexcept
        {
            return as_mutable(temp_buffer);
        }

        [[nodiscard]]
        constexpr os::const_module_handle_t app() const noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr pxsize2d user_sizes() const noexcept
        {
            return user_sizes_cache;
        }
    };
}