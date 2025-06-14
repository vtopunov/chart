#pragma once

#include <debug/debug.h>

#include <ui/fwd.h>


namespace ui
{
    void quit() noexcept;

    [[nodiscard]]
    error_code_t error_code() noexcept;

    template<class FormatString, class... Args>
    void ui_fatal_debug(const FormatString& format_string, const Args&... args) noexcept
    {
        ::fatal_debug(format_string, args...);
        quit();
    }
}

using ui::ui_fatal_debug;