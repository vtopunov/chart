#pragma once

#include <egl_ui/egl_window_builder.h>

#include <widget/window.h>


namespace widget
{
    class window_builder : public egl_ui::egl_window_gatherer<window_builder>
    {
        static constexpr auto dialog_color = 0xf0f0f0_glrgb;

    public:
        constexpr window_builder() noexcept
        {
            background(dialog_color);
        }

        [[nodiscard]]
        window build() const noexcept
        {
            return create_egl_window(params());
        }
    };

}