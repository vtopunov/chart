#pragma once

#include <chrono>

#include <core/buffer_view.h>

#include <egl_ui/egl_window.h>

#include <widget/window_fwd.h>
#include <widget/shaders.h>


namespace widget
{
    struct window_construct_t
    {};

    constexpr window_construct_t window_construct{};

    struct window : public egl_window
    {
        shaders shaders{};
        buffer_t temp_buffer{};

#ifdef D_OS_WINDOWS
        pxsize2d user_sizes_cache{};
        std::chrono::steady_clock::time_point redraw_time_cache{};
#endif

        constexpr window(window_construct_t, egl_window&& egl) noexcept
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
            return D_CONDITIONAL_OS_WINDOWS(user_sizes_cache, sizes(*this));
        }
    };
}