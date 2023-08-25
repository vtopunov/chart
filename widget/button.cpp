#include "button.h"

#include <ui/event.h>

#include <widget/window_configation.h>
#include <widget/event.h>
#include <widget/text.h>


namespace widget
{
    namespace
    {
        [[nodiscard]] constexpr bool contains(const pxrectangle& r, const ui::pointer_event& p) noexcept
        {
            constexpr auto contains1d = []
            (
                ui::pointer_event::value_type p,
                pxrectangle::value_type p0,
                pxrectangle::size_type dp
            ) noexcept
            {
                return p >= p0 && p < (p0 + dp);
            };

            return contains1d(p.x(), r.x(), r.width())
                && contains1d(p.y(), r.y(), r.height());
        }

        struct button_colors
        {
            gl::rgba_colorf_t frame;
            gl::rgba_colorf_t body;

            [[nodiscard]]
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

        [[nodiscard]]
        constexpr pxrectangle rectangle_without_frame(const pxrectangle& r)noexcept
        {
            constexpr pxsize2d frame_sizes{ 1_px, 1_px };
            return
            {
                .position{ r.position + frame_sizes },
                .sizes{ r.sizes - 2u * frame_sizes }
            };
        }

        [[nodiscard]]
        constexpr font::fixed_point2d center(const pxrectangle& r) noexcept
        {
            return font::cursor::instance(r.position) + font::cursor::instance(r.sizes) / 2;
        }

        constexpr bool update_state(button_state& state, const button_state new_state) noexcept
        {
            const auto is_update = new_state != state;
            if (is_update)
            {
                state = new_state;
            }

            return is_update;
        }
    }

    event_result button::operator () (const ui::mouse_down_event& e) noexcept
    {
        if (contains(geometry, e))
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
        if (button_state::pressed == state)
        {
            const auto is_clicked = contains(geometry, e);

            state = D_CONDITIONAL_OS_WINDOWS(((is_clicked) ? button_state::hovered : button_state::free), button_state::free);

            if (is_clicked && clicked)
            {
                clicked();
            }

            return event_result::redraw;
        }

        return event_result::idle;
    }

#ifdef D_OS_WINDOWS
    event_result button::operator () (const ui::mouse_move_event& e) noexcept
    {
        if (button_state::pressed != state)
        {
            const auto new_state = contains(geometry, e) ? button_state::hovered : button_state::free;
            if (update_state(state, new_state))
            {
                return event_result::redraw;
            }
        }

        return event_result::idle;
    }

#endif

    window_configation button::operator()(const init_event&) const noexcept
    {
        return enable_gray_texture_mix_color_shdr
            | enable_colored_rectangle_shdr
            | enable_pix8_temp_buffer;
    }

    void button::operator()(const redraw_event& e) noexcept
    {
        const auto colors = button_colors::instance(state);

        e.shaders.colored_rectangle.draw(geometry, colors.frame);

        const auto client_rc = rectangle_without_frame(geometry);
        e.shaders.colored_rectangle.draw(client_rc, colors.body);

        if (text::draw_to_cache(*this, e, client_rc.sizes)) [[likely]]
        {
            const auto client_rc_center = center(client_rc);
            const auto texture_center = text_cache.center();
            const auto position = client_rc_center - texture_center;

            const point2d px_position
            {
                font::ceil_to<pxside_t>(position.x()),
                font::ceil_to<pxside_t>(position.y())
            };

            e.shaders.gray_texture_mix_color.draw(px_position, text_cache.texture(), gl::colors::black_f);
        }
    }
}