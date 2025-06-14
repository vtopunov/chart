#include <debug/debug.h>

#include <px/pixmap.h>

#include <widget/stretchable.h>
#include <widget/run.h>

#include <chart/grid_shader.h>


namespace
{
    using glsl_float_t = float;

    namespace glsl
    {
        using std::min;
        using std::max;

        [[nodiscard]]
        glsl_float_t floor(glsl_float_t x) noexcept
        {
            return static_cast<glsl_float_t>(std::floor(static_cast<doublemax_t>(x)));
        }

        [[nodiscard]]
        glsl_float_t mod(glsl_float_t x, glsl_float_t y) noexcept
        {
            return x - y * floor(x / y);
        }

        const glsl_float_t width = 1.0f;
        const auto eps = static_cast<glsl_float_t>(chart::shader::frag::grid::eps);

        [[nodiscard]]
        bool grid_helper(glsl_float_t y, glsl_float_t period) noexcept
        {
            const auto t = mod(y, period) - width;
            return t < eps;
        }

        [[nodiscard]]
        bool grid(glsl_float_t y, glsl_float_t period) noexcept
        {
            const auto out_of_gridline = y + width;
            const auto b_grid = grid_helper(y, period);
            const auto b_out_of_grid = grid_helper(out_of_gridline, period);
            return b_grid && !b_out_of_grid;
        }
    }

    [[nodiscard]]
    gl::texture2d grid_rendering(glsl_float_t period, pixspan<rgba_color> image) noexcept
    {
        zero_memory(image);

        {
            npx_t grid_index{ 0 };

            for (npx_t index = image.height(); index; )
            {
                --index;

                const auto f_y = (image.height() - index) - 0.5f;
                const glsl_float_t y = f_y;

                if (glsl::grid(y, period))
                {
                    fill(image.line(index), colors::green);

                    const auto grid_fpx = grid_index * period;
                    const auto grid_px = image.height() - index - 1_npx;
                    const auto grid_err = abs(grid_fpx - grid_px);
                    D_ASSERT(grid_err <= (0.5 + glsl::eps));
                    D_UNUSED(grid_err);
                    ++grid_index;
                }
            }
        }

        return gl::create_texture2d(image);
    }

    [[nodiscard]]
    inline double zoom(double value, double rot) noexcept
    {
        constexpr double mul{ 0.05 };
        const auto sign = 1 - 2 * std::signbit(rot);
        return value + sign * mul * abs(rot);
    }

    constexpr widget::stretchable_pxrectangle geometry{ /*.position{20_npx, 75_npx}, .sizes{-20_pxoff, -20_pxoff}*/ };


    struct processor
    {
        shader_embed::default_texture shaders{};
        px::pixmap<rgba_color> image{};
        gl::texture2d texture{};
        glsl_float_t period{ 39.95f /*39.8475494 40.250049755960461f /*60.046324227548830f*/ };
        bool need_redraw{ false };

        bool operator () (widget::basic_initialization_event<>) noexcept
        {
            if (shaders.load())
            {
                shaders.position(geometry.position);
                return true;
            }

            return false;
        }

        void operator () (widget::viewport_event<> e) noexcept
        {
            shaders.use();
            shaders.viewport(e.viewport());
        }

        widget::event_result operator () (const ui::mouse_wheel_event& e) noexcept
        {
            period = static_cast<glsl_float_t>(zoom(period, e.rot()));
            texture.reset();
            return widget::event_result::redraw;
        }

        void operator () (widget::redraw_event<> e) noexcept
        {
            shaders.use();

            if (const auto new_sizes = widget::stretchable_sizes(geometry, e);  new_sizes != sizes(texture))
            {
                image = { image.release_buffer(), new_sizes };
                if (!image)
                {
                    ui_fatal_debug("grid emu: out of memory");
                    return;
                }

                texture = grid_rendering(period, image);
                shaders.texture(texture);
            }

            shaders.draw();
        }

        constexpr dummy apply(no_overload) const noexcept
        {
            return dummy_v;
        }
    };

}

int main() noexcept
{
    auto window = widget::window_builder{}
        .sizes(ui::adjust_sizes(480_npx, 960_npx))
        .command_show(ui::show_command::normal)
        .build();


    return widget::run<processor>(window);
}




