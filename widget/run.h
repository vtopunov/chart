#pragma once

#include <debug/debug.h>

#include <gl/draw.h>

#include <widget/context.h>
#include <widget/event_matching.h>


namespace widget
{
    namespace private_detail_run
    {
        template<class ER>
        [[nodiscard]] constexpr std::enable_if_t<std::negation_v<has_event_result<ER>>, bool> initialization_was_successful(event_result_processor<ER>) noexcept
        {
            return true;
        }

        [[nodiscard]] constexpr bool initialization_was_successful(event_result_processor<bool> result) noexcept
        {
            return result.result;
        }

        [[nodiscard]] constexpr bool initialization_was_successful(event_result_processor<event_result> result) noexcept
        {
            return e_bit_check(result.result, event_result::invalid);
        }

        template<class Widget, class Event>
        [[nodiscard]] bool apply_initialization_event(Widget& widget, const Event& e) noexcept
        {
            return initialization_was_successful(apply_event(widget, e));
        }

        class paining_owner
        {
        public:
            explicit paining_owner(const widget::window& w) noexcept
                : owner_{ w }
            {
                gl::viewport(viewport(w));
                gl::clear(colors::dialog_color_f);
            }

        private:
            egl_painting_owner owner_;
        };

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
                    apply_ui_event(e);
                }

                return std::nullopt;
            }
#endif

            void operator () (ui::content_rect_changed_event) noexcept
            {
                if (ref_window().update_viewport())
                {
                    if (!apply_ui_initialization_event(viewport_event_base_v))
                    {
                        ui_fatal_debug(cref_window(), "update viewport error: ui error: {}, egl error: {}",
                            ui::error_code(), eglGetError());
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
                apply_ui_event(e);
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
                    draw();
                }

#else
                if (e_bit_extract(combined_event_result_, event_result::redraw))
                {
                    draw();
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
            bool initialize() noexcept
            {
                return apply_ui_initialization_event(initialization_event_base_v)
                    && apply_ui_initialization_event(viewport_event_base_v);
            }

        private:
            template<class Event>
            bool apply_ui_initialization_event(const Event& e) noexcept
            {
                const event_common_context e_cc{ e, common_context_ };
                return apply_initialization_event(common_context_, e_cc)
                    && apply_initialization_event(widget_, e_cc);
            }

            template<class Event>
            void apply_ui_event(const Event& e) noexcept
            {
                const event_common_context e_cc{ e, common_context_ };
                combine_event_result(combined_event_result_, apply_event(common_context_, e_cc));
                combine_event_result(combined_event_result_, apply_event(widget_, e_cc));
            }

            void draw() noexcept
            {
                [[maybe_unused]] const paining_owner painting_owner{ cref_window() };
                const event_common_context e_cc{ redraw_event_base_v, common_context_ };
                D_UNUSED(apply_event(widget_, e_cc));
            }

        private:
            Widget widget_;
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
            e_debug
            (
                "create window error: window error: {}, egl error: {}",
                ui::error_code(),
                eglGetError()
            );
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
                "widget's initialize error: window error: {}, egl error: {}",
                ui::error_code(),
                eglGetError()
            );
            return EXIT_FAILURE;
        }

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