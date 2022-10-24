#pragma once

#include <functional>
#include <string>

#include <ui/event_fwd.h>

#include <gl/texture.h>

#include <utility/font_cache.h>

#include <widget/window_fwd.h>
#include <widget/event_result.h>

namespace widget
{
    enum class button_state
    {
        free,
        hovered,
        pressed
    };

    struct button
    {
        pxrectangle geometry{};
        std::u8string text{};
        std::function<void()> clicked{};
        font_cache::face font{};
        gl::texture2d texture_text_cache{};
        button_state state{ button_state::free };

        void set_text(std::u8string new_text) noexcept
        {
            if (new_text != text)
            {
                text = std::move(new_text);
                texture_text_cache.reset();
            }
        }

        bool initialize(window& w) noexcept;

        event_result operator () (const ui::mouse_down_event& e) noexcept;
        event_result operator () (const ui::mouse_up_event& e) noexcept;
        event_result operator () (const ui::mouse_move_event& e) noexcept;
        
        void draw(const window& w) noexcept;
    };
}
