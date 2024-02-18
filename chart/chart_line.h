#pragma once

#include <core/small_vector.h>

#include <px/algorithm.h>

#include <gl/color.h>
#include <gl/texture.h>

#include <widget/shader.h>

#include <chart/fwd.h>


namespace chart
{
    [[nodiscard]]
    constexpr space_diagonal_t space_diagonal_with(const space_diagonal_t& dia, const real_point2d& pt) noexcept
    {
        return
        {
            md_min(dia._0, pt),
            md_max(dia._1, pt)
        };
    }

    constexpr void update_line_space_diagonal(space_diagonal_t& diagonal, real_point2d_cspan line) noexcept
    {
        for (const auto& pt : line) [[likely]]
        {
            if (md_isfinite(pt)) [[likely]]
            {
                diagonal = space_diagonal_with(diagonal, pt);
            }
        }
    }

    template<class Points>
    struct basic_chart_line
    {
        using points_type = Points;

        points_type points{};
        gl::rgba_colorf_t pen_color{ gl::colors::red_f };
        gl::unique_texture2d_resource texture_cache{};

        void set_points(points_type new_points) noexcept
        {
            points = std::move(new_points);
            texture_cache.reset();
        }

        void clear() noexcept
        {
            points = null_v<points_type>;
            texture_cache.reset();
        }

        void operator () (space_diagonal_t& diagonal) const noexcept
        {
            update_line_space_diagonal(diagonal, points);
        }

        void operator()(pix8span pixs, transformation_t value2px) noexcept
        {
            zero_memory(pixs);
            px::draw_polyline(pixs, points, value2px);
            D_ASSERT_OR_UNUSED(gl::update(texture_cache, pixs));
        }

        template<class VS, class FS>
        void operator()(const widget_shader_user<VS, FS>& shdr) const noexcept
        {
            shdr.store(pen_color)
                .store(texture_cache)
                .draw();
        }
    };

    using chart_line_vpoint_t = small_vector<real_point2d>;
    using chart_line = basic_chart_line<chart_line_vpoint_t>;
    using chart_spanline = basic_chart_line<real_point2d_cspan>;
}

using chart::basic_chart_line;
using chart::chart_line_vpoint_t;
using chart::chart_line;
using chart::chart_spanline;