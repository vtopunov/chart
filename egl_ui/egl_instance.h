#pragma once

#include <core/resouce.h>

#include <egl_ui/egl_resources.h>

namespace egl_ui
{
    struct egl_resources_collector
    {
        void operator () (const egl_resources& egl) const noexcept;
    };

    using egl_t = unique_resource<egl_resources, egl_resources_collector>;

    [[nodiscard]]
    egl_t egl_instance(os::module_handle_t module) noexcept;

#ifdef D_OS_WINDOWS
    inline egl_t egl_instance() noexcept
    {
        return egl_instance(nullptr);
    }

#endif
}

using egl_ui::egl_instance;
using egl_ui::egl_t;