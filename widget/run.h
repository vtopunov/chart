#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <widget/context.h>
#include <widget/event_matching.h>


namespace widget
{
    namespace private_detail_run
    {
        namespace initialization
        {
            template<class ER>
            [[nodiscard]] constexpr std::enable_if_t<std::negation_v<has_event_result<ER>>, bool> was_successful(event_result_processor<ER>) noexcept
            {
                return true;
            }

            [[nodiscard]] constexpr bool was_successful(event_result_processor<bool> result) noexcept
            {
                return result.result;
            }

            [[nodiscard]] constexpr bool was_successful(event_result_processor<event_result> result) noexcept
            {
                return e_bit_check(result.result, event_result::invalid);
            }

            template<class Widget, class Event>
            [[nodiscard]] bool apply_event(Widget& widget, const Event& e) noexcept
            {
                return was_successful(widget::apply_event(widget, e));
            }
        }

        template<class Widget>
        class processor
        {
        public:
            template<class... Args>
            explicit processor(widget::window& window, Args&&... args) noexcept
                : widget_{ std::forward<Args>(args)... }
                , common_context_{ window }
            {}

#ifdef D_OS_WINDOWS
            std::nullopt_t operator () (const ui::size_event& e) noexcept
            {
                const auto new_size = e.sizes();
                if (new_size.width() > 0_npx && new_size.height() > 0_npx) [[likely]]
                {
                    D_ASSERT(new_size.width() <= cref_window().viewport().width());
                    D_ASSERT(new_size.height() <= cref_window().viewport().height());
                    ref_window().content_sizes(new_size);
                    _apply_event(e);
                }

                return std::nullopt;
            }
#endif

            void operator () (ui::content_rect_changed_event) noexcept
            {
                if (ref_window().update_viewport())
                {
                    if (!setup_viewport()) [[unlikely]]
                    {
                        ui_fatal_debug(cref_window(), "update viewport error: ui: {}", egl_ui::error_code());
                    }
                }
            }

            void operator () (ui::redraw_needed_event) noexcept
            {
                e_bit_or_eq(combined_event_result_, widget::event_result::redraw);
            }

            template<ui::event_style Style>
            std::nullopt_t operator () (const ui::specialized_event<Style>& e) noexcept
            {
                _apply_event(e);
                return std::nullopt;
            }

            ui::milliseconds operator () (ui::idle_event) noexcept
            {
#ifdef D_OS_WINDOWS
                using namespace std::chrono_literals;
                constexpr std::chrono::steady_clock::duration min_update_time{ 35ms };

                if (e_bit_check(combined_event_result_, event_result::redraw))
                {
                    const auto now = std::chrono::steady_clock::now();

                    if ((now - redraw_time_cache) < min_update_time)
                    {
                        return ui::milliseconds::zero();
                    }

                    e_bit_clear(combined_event_result_, event_result::redraw);
                    redraw_time_cache = now;
                    _draw();
                }

#else
                if (e_bit_extract(combined_event_result_, event_result::redraw))
                {
                    _draw();
                }

#endif

                return ui::infinite;
            }

            [[nodiscard]]
            constexpr const window& cref_window() const noexcept
            {
                return common_context_.cref_window();
            }

            [[nodiscard]]
            constexpr window& ref_window() noexcept
            {
                return common_context_.ref_window();
            }

            [[nodiscard]]
            bool initialize_widget() noexcept
            {
                return _apply_initialization_event(initialization_event_base_v);
            }

            [[nodiscard]]
            bool setup_viewport() noexcept
            {
                if (_apply_initialization_event(viewport_event_base_v)) [[likely]]
                {
                    gl::viewport(cref_window().viewport());
                    return true;
                }

                return false;
            }

            [[nodiscard]]
            bool initialize() noexcept
            {
                return initialize_widget() && setup_viewport();
            }

        private:
            template<class Event>
            bool _apply_initialization_event(const Event& e) noexcept
            {
                const event_common_context e_cc{ e, common_context_ };
                return initialization::apply_event(common_context_, e_cc)
                    && initialization::apply_event(widget_, e_cc);
            }

            template<class Event>
            void _apply_event(const Event& e) noexcept
            {
                const event_common_context e_cc{ e, common_context_ };
                combine_event_result(combined_event_result_, apply_event(common_context_, e_cc));
                combine_event_result(combined_event_result_, apply_event(widget_, e_cc));
            }

            void _draw() noexcept
            {
                [[maybe_unused]] egl_painting_owner painting_owner{ cref_window() };
                gl::clear();

                const event_common_context e_cc{ redraw_event_base_v, common_context_ };
                D_UNUSED(apply_event(widget_, e_cc));
            }

        private:
            D_NO_UNIQUE_ADDRESS Widget widget_;
            common_context_t<Widget> common_context_{};
            D_ONLY_OS_WINDOWS(std::chrono::steady_clock::time_point redraw_time_cache{});
            event_result combined_event_result_{ event_result::redraw };
        };
    }

    template<class Widget, class... Args>
    int run(widget::window& window, Args&&... args) noexcept
    {
        if (!window) [[unlikely]]
        {
            e_debug("create window error: {}", egl_ui::error_code());
            return EXIT_FAILURE;
        }

            private_detail_run::processor<Widget> processor
        {
            window,
            std::forward<Args>(args)...
        };

        if (!processor.initialize()) [[unlikely]]
        {
            e_debug
            (
                "main widget initialize error: {}",
                egl_ui::error_code()
            );
            return EXIT_FAILURE;
        }

        gl::clear_color(colors::dialog_color_f);

        return ui::run_event_loop(processor.cref_window(), processor);
    }

    template<class Widget, class... Args>
    int run(os::module_handle_t app, Args&&... args) noexcept
    {
        auto window = widget::window_builder{}
            .module(app)
            .build();

        return run<Widget>(window, std::forward<Args>(args)...);
    }
}