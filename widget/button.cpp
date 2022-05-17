#include "button.h"

#include <debug/debug.h>

#include <widget/event_context.h>
#include <widget/shader.h>
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

        bool button_shaders_initialize(px::size2d viewport_sizes) noexcept
        {
            return shader::colored_rectangle::initialize(viewport_sizes)
                && shader::gray_texture_mix_color::initialize(viewport_sizes);
        }

        constexpr rectangle rectangle_without_frame(const rectangle& r)noexcept
        {
            constexpr size2d frame_sizes{ 1_px, 1_px };
            return
            {
                .position{ r.position + frame_sizes },
                .sizes{ r.sizes - 2 * frame_sizes }
            };
        }

        constexpr void update_state_and_redraw(button& b, event_context& context, button_state new_state) noexcept
        {
            if (b.state != new_state)
            {
                b.state = new_state;
                context.need_redraw = true;
            }
        }
    }

    bool button::initialize(px::size2d viewport_sizes) noexcept
    {
        if (!button_shaders_initialize(viewport_sizes))
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

    void button::operator () (event_context& context, const ui::mouse_lbutton_down_event& e) noexcept
    {
        if (geometry.contains(e.position()))
        {
            update_state_and_redraw(*this, context, button_state::pressed);
        }
    }

    void button::operator () (event_context& context, const ui::mouse_lbutton_up_event& e) noexcept
    {
        if (state == button_state::pressed)
        {
            const auto is_clicked = geometry.contains(e.position());

            const auto new_state = (is_clicked) ? button_state::hovered : button_state::free;

            if (is_clicked && clicked)
            {
                clicked();
            }

            update_state_and_redraw(*this, context, new_state);
        }
    }

    void button::operator () (event_context& context, const ui::mouse_move_event& e) noexcept
    {
        if (state != button_state::pressed)
        {
            const auto new_state = geometry.contains(e.position()) ? button_state::hovered : button_state::free;
            update_state_and_redraw(*this, context, new_state);
        }
    }

    void button::draw(buffer_t& temp_buffer) noexcept
    {
        const auto colors = button_colors::instance(state);

        shader::colored_rectangle::draw(geometry, colors.frame);

        const auto client_rc = rectangle_without_frame(geometry);
        shader::colored_rectangle::draw(client_rc, colors.body);

        if (!text_texture && !text.empty())
        {
            text_texture = text::draw_to_texture(temp_buffer, font, text, client_rc.sizes);
        }

        if (text_texture)
        {
            const auto position = (2 * client_rc.position + client_rc.sizes - sizes(text_texture)) / 2;
            shader::gray_texture_mix_color::draw(position, text_texture, gl::colors::black_f);
        }
    }
}