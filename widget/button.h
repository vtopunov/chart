#pragma once

#include <functional>
#include <string>

#include <widget/fwd.h>
#include <widget/event.h>
#include <widget/shader.h>
#include <widget/text.h>


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
        text::drawing_cache text_cache{};
        button_state state{ button_state::free };

        void set_text(std::u8string new_text) noexcept
        {
            if (new_text != text)
            {
                text = std::move(new_text);
                text_cache.clear();
            }
        }

        event_result operator () (const ui::mouse_down_event& e) noexcept;
        event_result operator () (const ui::mouse_up_event& e) noexcept;

#ifdef D_OS_WINDOWS
        event_result operator () (const ui::mouse_move_event& e) noexcept;
#endif       

        using redraw_event_type = redraw_event<
            shader::gray_texture_mix_color,
            shader::colored_rectangle,
            buffer_view
        >;

        void operator () (redraw_event_type e) noexcept;

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }
    };
}
