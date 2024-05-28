#pragma once

#include <string>

#include <utility/font_cache.h>

#include <widget/ex_context.h>
#include <widget/shader.h>
#include <widget/text.h>


namespace widget
{
    struct label
    {
        pxpoint2d position{};
        std::u8string text{};
        font_cache::face font{};
        text_cache text_cache{};

        void set_text(std::u8string new_text) noexcept
        {
            if (new_text != text)
            {
                text = std::move(new_text);
                text_cache.clear();
            }
        }

        [[nodiscard]] bool operator () (widget::basic_initialization_event<>) noexcept;

        using redraw_event_type = basic_redraw_event<
            shader::luminance_texture_mix_color,
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
