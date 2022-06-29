#include <widget/label.h>
#include <widget/button.h>
#include <widget/run.h>


namespace
{
    using namespace widget;

    struct main_widget
    {
        button b0
        {
            .geometry
            {
                .position{30_px, 50_px},
                .sizes{150_px, 50_px}
            },
            .text{ u8"Button №1" }
        };

        button b1
        {
            .geometry
            {
                .position{30_px, 150_px},
                .sizes{150_px, 50_px}
            },
            .text{ u8"Button №2" },
        };

        button b2
        {
            .geometry
            {
                .position{30_px, 250_px},
                .sizes{150_px, 50_px}
            },
            .text{ u8"Button №3" },
        };

        button exit_b
        {
            .geometry
            {
                .position{30_px, 350_px},
                .sizes{150_px, 50_px}
            },
            .text{ u8"Exit" },
            .clicked{ ui::quit  }
        };

        label lb
        {
            .position{220_px, 220_px},
            .text{ u8"Label" },
        };

        main_widget() noexcept
        {
            b0.clicked = [this]() noexcept
            {
                clicked(this->b0);
            };

            b1.clicked = [this]() noexcept
            {
                clicked(this->b1);
            };

            b2.clicked = [this]() noexcept
            {
                clicked(this->b2);
            };
        };

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

int app_main(os::module_handle_t) noexcept
{
    return widget::run<main_widget>();
}




