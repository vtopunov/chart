#pragma once

#include <core/resource.h>

#include <egl_ui/egl_window_resource.h>

namespace egl_ui
{
    struct display_surface
    {
        display_descriptor_t display;
        surface_descriptor_t surface;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!surface;
        }
    };

    struct egl_painting_collector
    {
        void operator () (display_surface surface) const noexcept
        {
            eglSwapBuffers(surface.display, surface.surface);
        }
    };

    class egl_painting_owner
    {
    public:
        constexpr egl_painting_owner(display_descriptor_t display, surface_descriptor_t surface) noexcept
            : lock_{ resource_construct, display, surface }
        {}

        egl_painting_owner(const egl_window_resource& egl) noexcept
            : egl_painting_owner{ egl.display, egl.surface }
        {
            glViewport
            (
                0, 0,
                narrow_cast<GLsizei>(width(egl)),
                narrow_cast<GLsizei>(height(egl))
            );

            clear(egl.background);
        }

    private:
        static void clear(const gl::rgba_colorf_t& c) noexcept
        {
            glClearColor(c.r, c.g, c.b, c.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }

    private:
        unique_resource<display_surface, egl_painting_collector> lock_;
    };
}

using egl_ui::egl_painting_owner;
