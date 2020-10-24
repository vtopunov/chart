#pragma once

#include <array>

#include <core/resouce.h>
#include <core/assert.h>

#include <display/window.h>
#include <display/egl/config.h>

namespace display
{
    struct egl_display_surface
    {
        EGLDisplay display;
        EGLSurface surface;
    };

    struct egl_display_surface_context
    {
        EGLDisplay display;
        EGLSurface surface;
        EGLContext context;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!context;
        }
    };

    struct egl_swap_buffers_collector
    {
        void operator () (egl_display_surface egl, resource_destroy_t) const noexcept
        {
            eglSwapBuffers(egl.display, egl.surface);
        }
    };

    using egl_end_t = unique_resource<egl_display_surface, egl_swap_buffers_collector>;

    struct egl_window_resource
    {
        using view_type = window_resource;

        window_resource app_wnd;
        window_resource renderer_wnd;

        egl_display_surface_context egl;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!egl;
        }

        [[nodiscard]]
        egl_end_t begin() const noexcept
        {
            const auto render_sizes = rect(renderer_wnd).sizes();

            glViewport
            (
                0,
                0,
                render_sizes.x(),
                render_sizes.y()
            );

            return
            {
                resource_construct,
                egl.display,
                egl.surface
            }; 
        }

        [[nodiscard]]
        constexpr operator window_resource() const noexcept
        {
            return app_wnd;
        }
    };

    using nullegl_t = null_t<egl_window_resource>;

    inline constexpr nullegl_t nullegl{};

    struct egl_window_resource_collector
    {
        void operator () (egl_window_resource egl, resource_destroy_t) const noexcept;
    };

    using egl_window_t = unique_resource<egl_window_resource, egl_window_resource_collector>;

    class egl_window_factory
    {
    public:
        egl_window_factory& title(std::wstring title) noexcept
        {
            app_.title(std::move(title));
            return *this;
        }

        egl_window_factory& window_type(unique_window_type_t type) noexcept
        {
            app_.type(std::move(type));
            return *this;
        }

        [[nodiscard]]
        egl_window_t create() noexcept;

    private:
        window_factory app_;
    };
}