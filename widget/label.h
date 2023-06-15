#pragma once

#include <string>

#include <ui/event_fwd.h>

#include <gl/texture.h>

#include <utility/font_cache.h>

#include <widget/widget_initializer.h>


namespace widget
{
    struct label
    {
        pxpoint2d position{};
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

        void operator () (widget_initializer& ini) const noexcept;

        void draw(const window& w) noexcept;
    };
}
