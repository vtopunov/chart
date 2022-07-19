#pragma once

#include <core/resouce.h>

#include <egl_ui/egl_resources.h>

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
        egl_painting_owner(const egl_resources& egl) noexcept
            : lock_{ resource_construct, egl.display, egl.surface }
        {
            glViewport
            (
                0, 0,
                narrow_cast<GLsizei>(width(egl)),
                narrow_cast<GLsizei>(height(egl))
            );
        }

    private:
        unique_resource<display_surface, egl_painting_collector> lock_;
    };
}

using egl_ui::egl_painting_owner;
