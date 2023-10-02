#pragma once

#include <egl_ui/app_ui_owner.h>
#include <egl_ui/egl_context.h>


namespace egl_ui
{
    struct egl_ui_owner : app_ui_owner
    {
        egl_context egl;

        constexpr explicit operator bool() const noexcept
        {
            return !!egl;
        }

        constexpr operator display_surface () const noexcept
        {
            return egl;
        }
    };

    using egl_ui_parameters = app_ui_parameters;

    [[nodiscard]]
    inline egl_ui_owner create_egl_ui(view_t<egl_ui_parameters> module) noexcept
    {
        egl_ui_owner result{ ui_startup_request(module) };
        if (const app_ui_owner& ui{ result }; ui) [[likely]]
        {
            result.egl = create_egl_context(render_window_handle(ui));
        }

        return result;
    }

    template<class Builder>
    using egl_ui_gatherer = app_ui_gatherer<Builder, egl_ui_parameters>;

    struct egl_ui_builder : egl_ui_gatherer<egl_ui_builder>
    {
        [[nodiscard]]
        egl_ui_owner build() const noexcept
        {
            return create_egl_ui(_c_params());
        }
    };

#ifdef D_OS_WINDOWS
    [[nodiscard]]
    inline egl_ui_owner create_egl_ui(ui::module_handle_t module) noexcept
    {
        return egl_ui_builder{}.module(module).build();
    }

#endif
}

using egl_ui::egl_ui_owner;
using egl_ui::egl_ui_builder;
using egl_ui::egl_ui_parameters;
using egl_ui::create_egl_ui;