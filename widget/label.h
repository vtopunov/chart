#pragma once

#include <string>

#include <ui/event_fwd.h>

#include <gl/texture.h>

#include <utility/font_cache.h>


namespace widget
{
    struct event_context;

    struct label
    {
        px::point2d position{};
        std::u8string text{};
        font_cache::face font{};
        gl::texture2d text_texture{};
    
        void set_text(std::u8string new_text) noexcept
        {
            if (new_text != text)
            {
                text = std::move(new_text);
                text_texture.reset();
            }
        }

        bool initialize(px::size2d viewport_sizes) noexcept;

        constexpr void operator () (event_context&, const ui::event&) const noexcept
        {}

        void draw(buffer_t& temp_buffer) noexcept;
    };
}
