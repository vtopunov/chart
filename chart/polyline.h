#pragma once

#include <core/small_vector.h>

#include <px/algorithm.h>

#include <gl/texture.h>

#include <chart/space_manipulation.h>
#include <chart/shader.h>


namespace chart
{
    using real_vpoint2d = small_vector<real_point2d>;

    [[nodiscard]]
    constexpr space_diagonal space_diagonal_with(const space_diagonal& dia, const real_point2d& pt) noexcept
    {
        return
        {
            md_min(dia._0, pt),
            md_max(dia._1, pt)
        };
    }

    constexpr void update_polyline_space_diagonal(space_diagonal& diagonal, real_point2d_cspan line) noexcept
    {
        for (const auto& pt : line) [[likely]]
        {
            if (md_isfinite(pt)) [[likely]]
            {
                diagonal = space_diagonal_with(diagonal, pt);
            }
        }
    }

    template<class Model>
    struct basic_polyline
    {
        using model_type = Model;
        using pen_type = rgbaf_color;
        static constexpr pen_type default_pen{ colors::black_f };

        model_type model{};
        pen_type pen{ default_pen };
        gl::unique_texture2d_resource texture_cache{};


        template<class M, std::enable_if_t<has_assignment_op_v<model_type, M>, int> = 0>
        void set_model(M&& new_model) noexcept
        {
            model = std::forward<M>(new_model);
            texture_cache.reset();
        }

        void reset_model() noexcept
        {
            model = null_v<model_type>;
            texture_cache.reset();
        }

        void operator () (space_diagonal& diagonal) const noexcept
        {
            update_polyline_space_diagonal(diagonal, model);
        }

        void operator()(const lumpixspan pixs, const space_manipulation& sys) noexcept
        {
            zero_memory(pixs);
            px::draw_polyline(pixs, model, make_transformation(sys));
            D_ASSERT_OR_UNUSED(gl::update(texture_cache, pixs));
        }

        void operator()(const shader::luminance_figure_user& shdr) const noexcept
        {
            shdr.color(pen)
                .texture(texture_cache)
                .draw();
        }
    };

    using polyline = basic_polyline<real_vpoint2d>;
    using polyspanline = basic_polyline<real_point2d_cspan>;
}