#pragma once

#include <core/buffer_view.h>

#include <egl_ui/egl_window.h>

#include <widget/window_fwd.h>
#include <widget/shaders.h>

namespace widget
{
    struct window
    {
        const egl_window egl;
        shaders shaders;
        buffer_t temp_buffer;
        pxsize2d user_sizes_cache;

        constexpr operator viewport_rectangle() const noexcept
        {
            return egl;
        }

        constexpr operator os::const_module_handle_t() const noexcept
        {
            return app();
        }

        constexpr operator buffer_view () const noexcept
        {
            return temp_buffer_view();
        }

        constexpr buffer_view temp_buffer_view() const noexcept
        {
            return as_mutable(temp_buffer);
        }

        constexpr os::const_module_handle_t app() const noexcept
        {
            return egl;
        }

        constexpr pxsize2d user_sizes() const noexcept
        {
            return user_sizes_cache;
        }
    };

    template<class T>
    constexpr auto is_cpmv_v = std::disjunction_v<
        std::is_copy_constructible<T>,
        std::is_copy_assignable<T>,
        std::is_move_constructible<T>,
        std::is_move_assignable<T>
    >;

    static_assert(!is_cpmv_v<window>);
}