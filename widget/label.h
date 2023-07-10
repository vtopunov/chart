#pragma once

#include <string>

#include <ui/event_fwd.h>

#include <gl/texture.h>

#include <widget/widget_initializer.h>
#include <widget/text.h>

namespace widget
{
    struct label
    {
        pxpoint2d position{};
        std::u8string text{};
        font_cache::face font{};
        text::drawing_cache text_cache{};
    
        void set_text(std::u8string new_text) noexcept
        {
            if (new_text != text)
            {
                text = std::move(new_text);
                text_cache.clear();
            }
        }

        void operator () (widget_initializer& ini) const noexcept;

        void draw(const window& w) noexcept;
    };
}
