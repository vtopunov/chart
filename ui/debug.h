#pragma once

#include <debug/debug.h>

#include <ui/app.h>

namespace ui
{
    template<class FormatString, class... Args>
    void fatal(os::const_module_handle_t app, const FormatString& format_string, const Args&... args) noexcept
    {
        fatal_debug(format_string, args...);
        ::ui::quit(app);
    }
}

using ui::fatal;