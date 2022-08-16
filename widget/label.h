#pragma once

#include <string>

#include <ui/event_fwd.h>

#include <gl/texture.h>

#include <utility/font_cache.h>

#include <widget/window_fwd.h>


namespace widget
{
    struct label
    {
        px::point2d position{};
        std::u8string text{};
        font_cache::face font{};
        gl::texture2d texture_text_cache{};
    
        void set_text(std::u8string new_text) noexcept
        {
            if (new_text != text)
            {
                text = std::move(new_text);
                texture_text_cache.reset();
            }
        }

        bool initialize(window& w) noexcept;

        void draw(const window& w) noexcept;
    };
}
