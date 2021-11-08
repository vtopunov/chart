#pragma once

#include <ui/window.h>
#include <egl/config.h>

namespace egl
{
    using ui_window_resource_t = ui::window_resource;

    struct _egl_descriptor
    {};

    template<class descriptor>
    using _egl_descriptor_t = std::conditional_t<std::is_class_v<std::remove_pointer_t<descriptor>>, std::remove_pointer_t<descriptor>, _egl_descriptor>;

    struct _display_descriptor : _egl_descriptor_t<EGLDisplay>
    {};

    struct _surface_descriptor : _egl_descriptor_t<EGLDisplay>
    {};

    struct _context_descriptor : _egl_descriptor_t<EGLContext>
    {};

    using display_descriptor = std::conditional_t<std::is_pointer_v<EGLDisplay>, _display_descriptor*, _display_descriptor>;

    using surface_descriptor = std::conditional_t<std::is_pointer_v<EGLSurface>, _surface_descriptor*, _surface_descriptor>;

    using context_descriptor = std::conditional_t<std::is_pointer_v<EGLContext>, _context_descriptor*, _context_descriptor>;

    struct display_surface
    {
        display_descriptor display;
        surface_descriptor surface;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!surface;
        }
    };

    struct display_surface_context
    {
        display_descriptor display;
        surface_descriptor surface;
        context_descriptor context;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!context;
        }
    };

    inline void swap_buffers(display_surface surface) noexcept
    {
        eglSwapBuffers(surface.display, surface.surface);
    }

    struct swap_buffers_collector
    {
        void operator () (display_surface surface, resource_destroy_t) const noexcept
        {
            swap_buffers(surface);
        }
    };

    using painting_context_t = unique_resource<display_surface, swap_buffers_collector>;

    struct window_resource
    {
        using view_type = ui_window_resource_t;

        ui_window_resource_t app_wnd;
        ui_window_resource_t renderer_wnd;

        size2d_t viewport;

        display_surface_context egl;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!egl;
        }

        struct painting_initializer
        {
            size2d_t viewport;
            display_surface surface;

            [[nodiscard]]
            constexpr explicit operator bool() const noexcept
            {
                return !!surface;
            }
        };

        [[nodiscard]]
        constexpr operator painting_initializer () const noexcept
        {
            return 
            { 
                .viewport{ viewport },
                .surface
                {
                    .display{ egl.display, },
                    .surface{ egl.surface }
                }
            };
        }

        [[nodiscard]]
        constexpr operator ui_window_resource_t () const noexcept
        {
            return app_wnd;
        }
    };

    struct window_resource_collector
    {
        void operator () (const window_resource& egl, resource_destroy_t) const noexcept;
    };

    struct window : unique_resource<window_resource, window_resource_collector>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator window_resource::painting_initializer () const noexcept
        {
            return resource();
        }
    };

    [[nodiscard]]
    inline painting_context_t begin_painting(window_resource::painting_initializer resource) noexcept
    {
        if (resource)
        {
            glViewport
            (
                0, 0,
                narrow_cast<GLsizei>(resource.viewport.width()),
                narrow_cast<GLsizei>(resource.viewport.height())
            );
        }

        return
        {
            resource_construct,
            resource.surface
        };
    }

    class window_factory
    {
    public:
        window_factory& title(std::wstring title) noexcept
        {
            app_.title(std::move(title));
            return *this;
        }

        window_factory& window_type(ui::unique_window_type_t type) noexcept
        {
            app_.type(std::move(type));
            return *this;
        }

        [[nodiscard]]
        window create() noexcept;

    private:
        ui::window_factory app_;
    };
}