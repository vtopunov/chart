#include "button.h"

#include <debug/debug.h>

#include <ui/event.h>

#include <widget/window.h>
#include <widget/text.h>


namespace widget
{
    namespace
    {
        struct button_colors
        {
            gl::rgba_colorf_t frame;
            gl::rgba_colorf_t body;

            static constexpr button_colors instance(button_state state) noexcept
            {
                switch (state)
                {
                    case button_state::hovered:
                        return
                        {
                            .frame{0x0078d7_glrgb},
                            .body{0xe5f1fb_glrgb}
                        };

                    case button_state::pressed:
                        return
                        {
                            .frame{0x005499_glrgb},
                            .body{0xcce4f7_glrgb}
                        };

                    default:
                        break;
                }

                return
                {
                    .frame{ 0xadadad_glrgb },
                    .body{ 0xe1e1e1_glrgb }
                };
            }
        };

        bool button_shaders_initialize(shaders& shaders, px::size2d viewport_sizes) noexcept
        {
            return shaders.colored_rectangle.initialize(viewport_sizes)
                && shaders.gray_texture_mix_color.initialize(viewport_sizes);
        }

        constexpr px::rectangle rectangle_without_frame(const px::rectangle& r)noexcept
        {
            constexpr size2d frame_sizes{ 1_px, 1_px };
            return
            {
                .position{ r.position + frame_sizes },
                .sizes{ r.sizes - 2 * frame_sizes }
            };
        }

        constexpr bool update_state(button_state& state, button_state new_state) noexcept
        {
            if (state != new_state)
            {
                state = new_state;
                return true;
            }

            return false;
        }
    }

    bool button::initialize(window& w) noexcept
    {
        if (!button_shaders_initialize(w.shaders, sizes(w)))
        {
            e_debug("shaders error: {}", glGetError());
            return false;
        }

        if (!font)
        {
            font = font_cache::load_font(text::default_font_name, text::default_font_size);
            if (!font)
            {
                e_debug("can't create font");
                return false;
            }
        }

        return true;
    }

    event_result button::operator () (const ui::mouse_down_event& e) noexcept
    {
        if (geometry.contains(e))
        {
            if (update_state(state, button_state::pressed))
            {
                return event_result::redraw;
            }
        }

        return event_result::idle;
    }

    event_result button::operator () (const ui::mouse_up_event& e) noexcept
    {
        switch (state)
        {
            case button_state::pressed:
            {
                const auto is_clicked = geometry.contains(e);

#ifdef D_OS_ANDROID
                state = button_state::free;
#else
                state = (is_clicked) ? button_state::hovered : button_state::free;
#endif

                if (is_clicked && clicked)
                {
                    clicked();
                }

                return event_result::redraw;
            }

#ifdef D_OS_ANDROID
            case button_state::hovered:
            {
                state = button_state::free;
                return event_result::redraw;
            }
#endif

            default:
                break;
        }

        return event_result::idle;
    }

    event_result button::operator () (const ui::mouse_move_event& e) noexcept
    {
        if (button_state::pressed != state)
        {
            const auto new_state = geometry.contains(e) ? button_state::hovered : button_state::free;
            if (update_state(state, new_state))
            {
                return event_result::redraw;
            }
        }

        return event_result::idle;
    }

    void button::draw(const window& w) noexcept
    {
        const auto colors = button_colors::instance(state);

        w.shaders.colored_rectangle.draw(geometry, colors.frame);

        const auto client_rc = rectangle_without_frame(geometry);
        w.shaders.colored_rectangle.draw(client_rc, colors.body);

        if (!texture_text_cache && !text.empty())
        {
            texture_text_cache = text::draw_to_texture(w.temp_buffer, font, text, client_rc.sizes);
        }

        if (texture_text_cache)
        {
            const auto position = (2 * client_rc.position + client_rc.sizes - sizes(texture_text_cache)) / 2;
            w.shaders.gray_texture_mix_color.draw(position, texture_text_cache, gl::colors::black_f);
        }
    }
}