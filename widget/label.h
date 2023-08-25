#pragma once

#include <string>

#include <gl/texture.h>

#include <widget/fwd.h>
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

        window_configation operator () (const init_event&) const noexcept;

        void operator () (const redraw_event& e) noexcept;
    };
}
