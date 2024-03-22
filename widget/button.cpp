#include "button.h"

#include <ui/event.h>


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
            rgbaf_color_t frame;
            rgbaf_color_t body;

            [[nodiscard]]
            static constexpr button_colors instance(button_state state) noexcept
            {
                using namespace color_literals;

                switch (state)
                {
                    case button_state::hovered:
                        return
                        {
                            .frame{0x0078d7_rgbf},
                            .body{0xe5f1fb_rgbf}
                        };

                    case button_state::pressed:
                        return
                        {
                            .frame{0x005499_rgbf},
                            .body{0xcce4f7_rgbf}
                        };

                    default:
                        break;
                }

                return
                {
                    .frame{0xadadad_rgbf},
                    .body{0xe1e1e1_rgbf}
                };
            }
        };

        [[nodiscard]]
        constexpr pxrectangle rectangle_without_frame(const pxrectangle& r)noexcept
        {
            constexpr pxsize2d frame_sizes{ 1_npx, 1_npx };
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

        [[nodiscard]]
        constexpr pxpoint2d ft_to_pxpoint2d(const vec2<font::fixed_t>& v) noexcept
        {
            return
            {
                font::ceil_to<pxsize_t>(v._0),
                font::ceil_to<pxsize_t>(v._1)
            };
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

    void button::operator()(redraw_event_type e) noexcept
    {
        const auto colors = button_colors::instance(state);
        const auto client_rc = rectangle_without_frame(geometry);

        {
            const auto& shdr = e.get<shader::colored_rectangle>().use();
            
            shdr.geometry(geometry)
                .color(colors.frame)
                .draw();

            shdr.geometry(client_rc)
                .color(colors.body)
                .draw();
        }

        if (text::draw_to_cache(*this, e.get<buffer_view>(), client_rc.sizes)) [[likely]]
        {
            const auto ft_position = center(client_rc) - text_cache.center();

            e.get<shader::luminance8_texture_mix_color>()
                .use()
                .position(ft_to_pxpoint2d(ft_position))
                .color(colors::black_f)
                .texture(text_cache.texture())
                .draw();
        }
    }
}