#pragma once

#include <core/resouce.h>

#include <egl_ui/ui_resources.h>

namespace egl_ui
{
    struct ui_resources_collector
    {
        void operator () (const ui_resources& ui) const noexcept;
    };

    using ui_wrapper = unique_resource<ui_resources, ui_resources_collector>;

    [[nodiscard]]
    ui_wrapper ui_intance(os::module_handle_t module) noexcept;
}