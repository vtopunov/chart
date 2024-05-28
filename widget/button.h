#pragma once

#include <functional>
#include <string>

#include <utility/font_cache.h>

#include <widget/ex_context.h>
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
        text_cache text_cache{};
        button_state state{ button_state::free };

        void set_text(std::u8string new_text) noexcept
        {
            if (new_text != text)
            {
                text = std::move(new_text);
                text_cache.clear();
            }
        }

        [[nodiscard]] bool operator () (widget::basic_initialization_event<>) noexcept;

        [[nodiscard]] event_result operator () (const ui::mouse_down_event& e) noexcept;
        [[nodiscard]] event_result operator () (const ui::mouse_up_event& e) noexcept;

#ifdef D_OS_WINDOWS
        [[nodiscard]] event_result operator () (const ui::mouse_move_event& e) noexcept;

#endif       

        using redraw_event_type = basic_redraw_event<
            shader::luminance_texture_mix_color,
            shader::colored_rectangle,
            buffer_view
        >;

        void operator () (redraw_event_type e) noexcept;

        template<class Fn>
        constexpr decltype(auto) apply(Fn&& fn) const noexcept
        {
            return ex_context_v<redraw_event_type>(std::forward<Fn>(fn));
        }
    };
}
