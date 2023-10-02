#include <variant>

#include <core/round.h>

#include <utility/shader_library.h>
#include <widget/run.h>

#include <pix8map_test_texture_generate.h>

using namespace std::chrono_literals;
using widget::event_result;


namespace
{
    pxoff2d mouse_pointer_distance_px(const ui::pointer_event::point2d_type& p0, const ui::pointer_event::point2d_type& p1) noexcept
    {
        const auto d_mouse = as_signed(p1 - p0);
        return
        {
            trunc_cast<pxoff_t>(d_mouse.x()),
            trunc_cast<pxoff_t>(d_mouse.y())
        };
    }

    [[nodiscard]]
    constexpr pxoff2d clamp_position(pxoff2d position, pxsize2d fig_sizes, pxsize2d viewport) noexcept
    {
        const auto min_position = -narrow2d<pxoff2d>(fig_sizes);
        const auto max_position = narrow2d<pxoff2d>(viewport);

        return
        {
            std::clamp(position.x(), min_position.x(), max_position.x()),
            std::clamp(position.y(), min_position.y(), max_position.y())
        };
    }

    class simple_widget
    {
        static constexpr auto invalid_mouse_pos = fill_to<point2d>(numeric_max_v<ui::pointer_event::value_type>);

    public:
        [[nodiscard]]
        bool operator () (const widget::window& w) noexcept
        {
            viewport_ = w.viewport;

            texture_ = pix8map_test_texture_generate(viewport_ / 2u);
            if (!texture_)
            {
                return false;
            }

            if (!shaders_.initialize(viewport_, texture_))
            {
                return false;
            }

            position_by_default();
            return true;
        }

        event_result operator () (const ui::mouse_double_click_event&) noexcept
        {
            mouse_trace_finish();
            position_by_default();
            return event_result::redraw;
        }

        event_result operator () (const ui::mouse_move_event& e) noexcept
        {
            if (!e.keys().is_left())
            {
                mouse_trace_finish();
                return event_result::idle;
            }

            if (1u == e.size())
            {
                const auto new_pos = e.pointer(0);
                if (invalid_mouse_pos != new_pos)
                {
                    const auto old_pos = std::exchange(mouse_pos_, new_pos);

                    if (invalid_mouse_pos != old_pos)
                    {
                        const auto d_mouse = mouse_pointer_distance_px(old_pos, new_pos);

                        const auto new_postion = clamp_position
                        (
                            position_ + d_mouse,
                            sizes(texture_),
                            viewport_
                        );

                        if (new_postion != position_)
                        {
                            position_ = new_postion;
                            return event_result::redraw;
                        }
                    }
                }
            }

            return event_result::idle;;
        }

        void operator () (const ui::mouse_up_event&) noexcept
        {
            mouse_trace_finish();
        } 

        void operator () (widget::redraw_event<>) const noexcept
        {
            shaders_.draw(position_);
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }

    private:
        constexpr void mouse_trace_finish() noexcept
        {
            mouse_pos_ = invalid_mouse_pos;
        }

        constexpr void position_by_default() noexcept
        {
            position_ = narrow2d<pxoff2d>((viewport_ - sizes(texture_)) / 2u);
        }

    private:
        class shaders_lib
        {
        public:
            bool initialize(pxsize2d viewport, gl::texture2d_resources texture) noexcept
            {
                if (!lib.build())
                {
                    return false;
                }

                lib.use();
                lib.frag.s_texture.store(texture);
                lib.frag.u_color.store(1.0f, 0.5f, 0.5f, 1.0f);
                lib.vert.u_viewport.store(viewport);
                lib.vert.u_size.store(sizes(texture));
                return true;
            }

            void draw(pxoff2d position) const noexcept
            {
                lib.use();
                lib.vert.u_position.store(position);
                lib.vert.a_frame.draw();
            }

        private:
            shader_library<vert::positioned_texture, frag::gray_texture_mix_color> lib{};
        };

        egl_ui::viewport_size2d viewport_{};
        shaders_lib shaders_{};
        gl::texture2d texture_{};
        ui::pointer_event::point2d_type mouse_pos_{ invalid_mouse_pos };
        pxoff2d position_{};
    };

    class main_widget
    {
    public:
        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(widget_);
        }

    private:
        simple_widget widget_{};
    };
}

int app_main(os::module_handle_t app) noexcept
{
    return widget::run<main_widget>(app);
}




