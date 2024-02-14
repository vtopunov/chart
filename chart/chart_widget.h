#pragma once

#include <core/small_vector.h>

#include <gl/color.h>
#include <gl/texture.h>

#include <widget/event.h>
#include <widget/shader.h>
#include <widget/user_gesture.h>

#include <chart/space_diagonal.h>


namespace chart
{
    using widget::content_size2d;
    using widget::stretchable_pxrectangle;
    using widget::event_result;

    namespace shader
    {
        using namespace widget::shader;
    }

    struct chart_line : intrusive_node_object<chart_line>
    {
        using points_type = small_vector<real_point2d>;

        points_type points{};
        gl::rgba_colorf_t pen_color{ gl::colors::red_f };
        gl::unique_texture2d_resource texture_cache{};
    };

    struct chart_widget
    {
        static constexpr auto background_color = gl::colors::white_f;

        stretchable_pxrectangle geometry{};
        space_diagonal_cache lines_space_cache{};
        intrusive_list<chart_line> lines{};
        pxsize2d chart_space_cache{};

        using mouse_wheel_event_type = widget::mouse_wheel_event<content_size2d>;

        using mouse_move_event_type = widget::mouse_move_event<
            ui::user_gesture,
            content_size2d
        >;

        using mouse_double_click_event_type = widget::mouse_double_click_event<
            content_size2d
        >;

        using redraw_event_type = widget::redraw_event<
            shader::gray_texture_mix_color,
            shader::colored_rectangle,
            buffer_view,
            content_size2d
        >;

        constexpr event_result operator () (const ui::size_event&) const noexcept
        {
            return event_result::redraw;
        }

        event_result operator () (mouse_wheel_event_type e) noexcept;
        event_result operator () (mouse_move_event_type e) noexcept;
        event_result operator () (mouse_double_click_event_type e) noexcept;

        void operator () (redraw_event_type e) noexcept;

        void clear_cache() noexcept
        {
            lines_space_cache.clear();
            chart_space_cache = {};
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn
            (
                widget::ex_context_v<redraw_event_type>,
                widget::ex_context_v<mouse_move_event_type>
            );
        }
    };
}

using chart::chart_line;
using chart::chart_widget;



