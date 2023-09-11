#include <widget/label.h>
#include <widget/button.h>
#include <widget/run.h>

using widget::button;
using widget::label;


namespace
{
    struct main_widget
    {
        button b0
        {
            .geometry
            {
                .position{30_px, 60_px},
                .sizes{150_px, 60_px}
            },
            .text{ u8"Button №1" }
        };

        button b1
        {
            .geometry
            {
                .position{30_px, 150_px},
                .sizes{150_px, 60_px}
            },
            .text{ u8"Button №2" },
        };

        button b2
        {
            .geometry
            {
                .position{30_px, 240_px},
                .sizes{150_px, 60_px}
            },
            .text{ u8"Button №3" },
        };

        button exit_b
        {
            .geometry
            {
                .position{30_px, 330_px},
                .sizes{150_px, 60_px}
            },
            .text{ u8"Exit" },
        };

        label lb
        {
            .position{30_px, 420_px},
            .text{ u8"Привет мир!" },
        };

        void operator () (os::const_module_handle_t app) noexcept
        {
            b0.clicked = [this] () noexcept
            {
                clicked(this->b0);
            };

            b1.clicked = [this] () noexcept
            {
                clicked(this->b1);
            };

            b2.clicked = [this] () noexcept
            {
                clicked(this->b2);
            };

            exit_b.clicked = [app] () noexcept
            {
                ui::quit(app);
            };
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(b0, b1, b2, exit_b, lb);
        }

        void clicked(const button& b) noexcept
        {
            lb.set_text(u8"Cliked: " + b.text);
        }
    };
}

int app_main(os::module_handle_t app) noexcept
{
    return widget::run<main_widget>(app);
}