#pragma once

#include <ui/debug.h>

#include <gl_core/draw.h>

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
            explicit processor(const widget::window& window, Args&&... args) noexcept
                : widget_{ std::forward<Args>(args)... }
                , context_{ window }
            {}

            void operator () (const ui::size_event& e) noexcept
            {
                const auto new_size = e.sizes();
                if (new_size.width() > 0_npx && new_size.height() > 0_npx) [[likely]]
                {
                    D_ASSERT(new_size.width() <= cref_window().viewport().width());
                    D_ASSERT(new_size.height() <= cref_window().viewport().height());
                    context_.ref_window().content_sizes(new_size);
                    _apply_event(e);
                }
            }

            void operator () (ui::content_rect_changed_event) noexcept
            {
                if (context_.ref_window().update_viewport())
                {
                    if (!setup_viewport()) [[unlikely]]
                    {
                        ui_fatal_debug("update viewport error: egli: {}", egli::error_code());
                    }
                }
            }

            void operator () (ui::redraw_needed_event) noexcept
            {
                e_bit_or_eq(combined_event_result_, widget::event_result::redraw);
            }

            template<ui::event_style Style>
            void operator () (const ui::specialized_event<Style>& e) noexcept
            {
                _apply_event(e);
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
                return context_.cref_window();
            }

            [[nodiscard]]
            bool initialize_widget() noexcept
            {
                return _apply_initialization_event(initialization_event_base_v);
            }

            [[nodiscard]]
            bool setup_viewport() noexcept
            {
                if (cref_window().viewport().has_positiven_mark()) [[likely]]
                {
                    if (_apply_initialization_event(viewport_event_base_v)) [[likely]]
                    {
                        gl::viewport(cref_window().viewport());
                        return true;
                    }
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
                const widget_event_factory e_cc{ e, context_ };
                return initialization::apply_event(context_, e_cc)
                    && initialization::apply_event(widget_, e_cc);
            }

            template<class Event>
            void _apply_event(const Event& e) noexcept
            {
                const widget_event_factory e_cc{ e, context_ };
                combine_event_result(combined_event_result_, ::widget::apply_event(context_, e_cc));
                combine_event_result(combined_event_result_, ::widget::apply_event(widget_, e_cc));
            }

            void _draw() noexcept
            {
                [[maybe_unused]]
                const egl_painting_owner painting_owner{ cref_window() };
                gl::clear();

                const widget_event_factory e_cc{ redraw_event_base_v, context_ };
                D_UNUSED(::widget::apply_event(widget_, e_cc));
            }

        private:
            D_NO_UNIQUE_ADDRESS Widget widget_;
            context_t<Widget> context_{};
            D_ONLY_OS_WINDOWS(std::chrono::steady_clock::time_point redraw_time_cache{});
            event_result combined_event_result_{ event_result::redraw };
        };

        template<class Widget, class... Args>
        int run_impl(const widget::window& window, Args&&... args) noexcept
        {
            if (!window) [[unlikely]]
            {
                e_debug("create window error: {}", egli::error_code());
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
                    egli::error_code()
                );
                return EXIT_FAILURE;
            }

            return ui::run_event_loop(processor.cref_window(), processor);
        }
    }

    template<class Widget, class... Args>
    int run(Args&&... args) noexcept
    {
        if constexpr (std::is_convertible_v<types_front_or_t<dummy, Args...>, const widget::window&>)
        {
            return private_detail_run::run_impl<Widget>(std::forward<Args>(args)...);
        }
        else
        {
            return private_detail_run::run_impl<Widget>(
                widget::window_builder{}.build(), 
                std::forward<Args>(args)...
            );
        }
    }
}