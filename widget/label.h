#pragma once

#include <string>

#include <widget/fwd.h>
#include <widget/event.h>
#include <widget/shaders.h>
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

        using redraw_event_type = redraw_event<
            shaders::gray_texture_mix_color,
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
