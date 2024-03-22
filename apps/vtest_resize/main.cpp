#include <random>
#include <variant>

#include <debug/debug.h>

#include <utility/shader_library.h>

#ifdef TEST_EGL_UI
#include <egl_ui/egl_ui_owner.h>
#endif

#ifdef TEST_WIDGET
#include <widget/run.h>
#endif

using namespace std::chrono_literals;

namespace
{
    class simple_widget
    {
        static constexpr auto invalid_mouse_pos = fill_to<point2d>(numeric_max_v<ui::pointer_event::value_type>);

    public:
        [[nodiscard]]
        bool initialize(pxsize2d viewport) noexcept
        {
            if (!lib_.build())
            {
                return false;
            }

            lib_.use();
            lib_.frag.u_color.store(colors::blue_f);
            lib_.vert.u_viewport.store(viewport);
            return true;
        }

        void update_content_sizes(pxsize2d content_sizes) noexcept
        {
            lib_.use();
            _store_content_sizes(content_sizes);
        }

        void draw() const noexcept
        {
            lib_.use();
            lib_.vert.a_frame.draw();
        }

#ifdef TEST_EGL_UI
        ui::milliseconds operator () (ui::idle_event) const noexcept
        {
            draw();
            return ui::infinite;
        }

#endif

#ifdef TEST_WIDGET
        bool operator () (viewport_size2d viewport) noexcept
        {
            return initialize(viewport);
        }

        widget::event_result operator () (const ui::size_event& e)
        {
            update_content_sizes(e.sizes());
            return widget::event_result::redraw;
        }

        void operator () (widget::redraw_event<>) noexcept
        {
            draw();
        }

        constexpr widget::noapply_t apply(no_overload) const noexcept
        {
            return widget::noapply;
        }

#endif

    private:
        void _store_content_sizes(pxsize2d content_sizes) noexcept
        {
            const auto rect_sizes = content_sizes / 2u;
            lib_.vert.u_position.store((content_sizes - rect_sizes) / 2u);
            lib_.vert.u_size.store(rect_sizes);
        }

    private:
        shader_library<vert::positioned_rectangle, frag::default_color> lib_{};
    };

#ifdef TEST_EGL_UI
    struct main_processor
    {
        const egl_ui_owner egl{};
        simple_widget widget{};

        bool initialize() noexcept
        {
            return widget.initialize(viewport(egl));
        }

        std::nullopt_t operator () (const ui::size_event& e) noexcept
        {
            widget.update_content_sizes(e.sizes());
            return std::nullopt;
        }

        ui::milliseconds operator () (ui::idle_event) const noexcept
        {
            const egl_painting_owner painting_owner{ egl };
            gl::viewport(viewport(egl));
            gl::clear(colors::white_f);
            widget.draw();
            return ui::infinite;
        }
    };

#endif

#ifdef TEST_WIDGET
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

#endif
}

int app_main(os::module_handle_t app) noexcept
{
#ifdef TEST_EGL_UI
    main_processor processor
    {
        .egl
        {
            egl_ui_builder{}
                .module(app)
                .sizes(300_npx, 300_npx)
                .command_show(ui::show_command::normal)
                .build()
        }
    };

    if (!processor.initialize())
    {
        e_debug("initialize error: window error: {}, egl error: {}\n",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    return ui::run_event_loop(processor.egl, processor);

#endif 

#ifdef TEST_WIDGET
    auto window = widget::window_builder{}
        .module(app)
        .sizes(300_npx, 300_npx)
        .command_show(ui::show_command::normal)
        .build();

    return widget::run<main_widget>(window);

#endif
}




