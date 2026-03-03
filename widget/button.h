#pragma once

#include <core/unique_function.h>

#include <shader/library.h>

#include <utility/font_cache.h>

#include <widget/ex_context.h>
#include <widget/event.h>
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
        unique_function<void()> clicked{};
        font_cache::cached_face font{ font_cache::default_font() };
        text_cache text_cache{};
        button_state state{ button_state::free };

        void set_text(std::u8string new_text) noexcept
        {
            text = std::move(new_text);
            text_cache.clear();
        }

        [[nodiscard]] event_result operator () (const ui::mouse_down_event& e) noexcept;
        [[nodiscard]] event_result operator () (const ui::mouse_up_event& e) noexcept;

#ifdef D_OS_WINDOWS
        [[nodiscard]] event_result operator () (const ui::mouse_move_event& e) noexcept;

#endif       

        using redraw_event_type = basic_redraw_event<
            const shader_embed::luminance_texture,
            const shader_embed::colored_rectangle,
            temp_byte_buffer
        >;

        void operator () (redraw_event_type e) noexcept;

        template<class Fn>
        constexpr decltype(auto) apply(Fn&& fn) const noexcept
        {
            return ex_context_v<redraw_event_type>(std::forward<Fn>(fn));
        }
    };
}
