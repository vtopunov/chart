#pragma once

#include <functional>
#include <string>

#include <ui/event.h>

#include <gl/texture.h>

#include <utility/font_cache.h>

#include <widget/rectangle.h>


namespace widget
{
    struct event_context;

    enum class button_state
    {
        free,
        hovered,
        pressed
    };

    struct button
    {
        rectangle geometry{};
        std::u8string text{};
        std::function<void()> clicked{};
        font_cache::face font{};
        gl::texture2d text_texture{};
        button_state state{ button_state::free };

        void set_text(std::u8string new_text) noexcept
        {
            if (new_text != text)
            {
                text = std::move(new_text);
                text_texture.reset();
            }
        }

        bool initialize(px::size2d viewport_sizes) noexcept;

        void operator () (event_context& context, const ui::mouse_lbutton_down_event& e) noexcept;
        void operator () (event_context& context, const ui::mouse_lbutton_up_event& e) noexcept;
        void operator () (event_context& context, const ui::mouse_move_event& e) noexcept;
        constexpr void operator () (event_context&, const ui::event&) const noexcept
        {}

        void draw(buffer_t& temp_buffer) noexcept;
    };
}
