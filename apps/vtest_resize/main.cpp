#include <debug/debug.h>

#include <shader/library.h>

#ifdef TEST_EGL_UI
#include <egli/egli_owner.h>
#endif

#ifdef TEST_WIDGET
#include <widget/run.h>
#endif


namespace
{
    class simple_widget
    {
    public:
        [[nodiscard]]
        bool initialize(pxsizes viewport) noexcept
        {
            if (lib_.load()) [[likely]]
            {
                lib_.color(colors::blue_f);
                lib_.viewport(viewport);
                return true;
            }

            return false;
        }

        void update_content_sizes(pxsizes content_sizes) const noexcept
        {
            lib_.use();
            _store_content_sizes(content_sizes);
        }

        void draw() const noexcept
        {
            lib_.use();
            lib_.draw();
        }

#ifdef TEST_EGL_UI
        ui::milliseconds operator () (ui::idle_event) const noexcept
        {
            draw();
            return ui::infinite;
        }

#endif

#ifdef TEST_WIDGET
        bool operator () (widget::viewport_event<> e) noexcept
        {
            return initialize(e.viewport());
        }

        widget::event_result operator () (const ui::size_event& e)
        {
            update_content_sizes(e.sizes());
            return widget::event_result::redraw;
        }

        void operator () (widget::redraw_event<>) const noexcept
        {
            draw();
        }

        constexpr dummy apply(no_overload) const noexcept
        {
            return dummy_v;
        }

#endif

    private:
        void _store_content_sizes(pxsizes content_sizes) const noexcept
        {
            const auto rect_sizes = content_sizes / 2u;
            lib_.position((content_sizes - rect_sizes) / 2u);
            lib_.sizes(rect_sizes);
        }

    private:
        shader_embed::colored_rectangle lib_{};
    };

#ifdef TEST_EGL_UI
    struct main_processor
    {
        const egli_owner egl{};
        simple_widget widget{};

        bool initialize() noexcept
        {
            return widget.initialize(egl.viewport());
        }

        std::nullopt_t operator () (const ui::size_event& e) noexcept
        {
            widget.update_content_sizes(e.sizes());
            return std::nullopt;
        }

        ui::milliseconds operator () (ui::idle_event) const noexcept
        {
            const egl_painting_owner painting_owner{ egl };
            gl::viewport(egl.viewport());
            gl::clear(colors::white_f);
            widget.draw();
            return ui::infinite;
        }
    };

#endif
}

int main() noexcept
{
    constexpr size2d sizes0{ 300_npx, 300_npx };

#ifdef TEST_EGL_UI
    main_processor processor
    {
        .egl
        {
            egli_builder{}
                .sizes(sizes0)
                .command_show(ui::show_command::normal)
                .build()
        }
    };

    if (!processor.initialize())
    {
        e_debug("initialize error: window error: {}, egl error: {}\n", egli::error_code());
        return EXIT_FAILURE;
    }

    return ui::run_event_loop(processor.egl, processor);

#endif 

#ifdef TEST_WIDGET
    auto window = widget::window_builder{}
        .sizes(sizes0)
        .command_show(ui::show_command::normal)
        .build();

    return widget::run<simple_widget>(window);

#endif
}




