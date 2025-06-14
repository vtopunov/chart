#pragma once

#include <shader/library.h>

#include <utility/font_cache.h>

#include <widget/ex_context.h>
#include <widget/event.h>
#include <widget/text.h>


namespace widget
{
    struct label
    {
        pxpoint position{};
        std::u8string text{};
        font_cache::cached_face font{ font_cache::default_font() };
        text_cache text_cache{};

        void set_text(std::u8string new_text) noexcept
        {
            text = std::move(new_text);
            text_cache.clear();
        }

        using redraw_event_type = basic_redraw_event<
            const shader_embed::luminance_texture,
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
