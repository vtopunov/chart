#include <widget/run.h>
#include <widget/stretchable.h>
#include <chart/grid.h>


namespace
{
    constexpr widget::stretchable_pxrectangle geometry{ /*.position{20_npx, 75_npx}, .sizes{-20_pxoff, -20_pxoff} */ };

    class simple_widget
    {
    public:
        bool operator () (widget::basic_initialization_event<>) noexcept
        {
            return lib_.load();
        }

        void operator () (widget::viewport_event<> e) noexcept
        {
            lib_.use();
            lib_.viewport(e.viewport());
        }

        constexpr widget::event_result operator () (const ui::size_event&) const noexcept
        {
            return widget::event_result::redraw;
        }

        void operator () (widget::redraw_event<> e) const noexcept
        {
            constexpr vec2 width{ 1_npx, 1_npx };
            constexpr vec2 begin{ 0.0, 0.0/*31.758812359218691*/ };
            constexpr vec2 repeat{ 84.03424182445018, 40.250049755960461 /*60.046324227548830*/ };

            lib_.use();
            lib_.position(geometry.position);
            lib_.sizes(widget::stretchable_sizes(geometry, e));
            lib_.color(colors::green_f);
            lib_.width(width);
            lib_.begin(begin);
            lib_.repeat(repeat);
            lib_.draw();
        }

        constexpr dummy apply(no_overload) const noexcept
        {
            return dummy_v;
        }

    private:
        chart::shader::grid lib_{};
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

int main() noexcept
{
    auto window = widget::window_builder{}
        .sizes(ui::adjust_sizes(480_npx, 960_npx))
        .command_show(ui::show_command::normal)
        .build();

    return widget::run<main_widget>(window);
}




