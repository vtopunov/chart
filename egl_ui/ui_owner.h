#pragma once

#include <ui/window.h>
#include <ui/event_loop.h>

#include <egl_ui/fwd.h>


namespace egl_ui
{
    namespace private_detail_ui_owner
    {
        template<class Processor, ui::event_style EventStyleSelector>
        struct event_match_one
        {
            static_assert(!std::is_reference_v<Processor>);

            Processor processor;

            ui::event_result_opt_t operator () (const ui::event& e) noexcept
            {
                if (EventStyleSelector == e.style())
                {
                    return ui::call_event(unrefwrap(processor), ui::event_for<EventStyleSelector>(e));
                }

                return std::nullopt;
            }
        };
    }

    using viewing_subwindow = D_OS_WINDOWS_OR(ui::window, dummy);

    class ui_owner
    {
    public:
        struct event_binder_type
        {
#ifdef D_OS_WINDOWS
            constexpr explicit event_binder_type(const ui_owner& ui) noexcept
                : app_event_source_{ ui.window() }
                , input_event_source_{ ui.viewing_window() }
            {}

            D_DISABLE_COPYMOVE_CA(event_binder_type);

            template<class EventTarget>
            [[nodiscard]] std::array<ui::event_processor, 2u> bind(EventTarget& target) const noexcept
            {
                using private_detail_ui_owner::event_match_one;

                auto target_ref = std::ref(target);

                return
                {
                    ui::create_event_processor
                    (
                        app_event_source_,
                        event_match_one
                        <
                            decltype(target_ref),
                            ui::event_style::size
                        >{ target_ref }
                    ),
                    ui::create_event_processor
                    (
                        input_event_source_,
                        ui::event_match{ target_ref }
                    )
                };
            }

        private:
            const window_handle_t app_event_source_;
            const window_handle_t input_event_source_;
#endif
        };


        constexpr ui_owner() noexcept = default;

        constexpr ui_owner
        (
            pxsizes viewport,
            module_handle_t module,
            ui::window&& initial_window,
            viewing_subwindow&& initial_viewing_window
        ) noexcept
            : viewport_{ viewport }
            , module_{ module }
            , window_{ std::move(initial_window) }
            , viewing_window_{ std::move(initial_viewing_window) }
        {}

        [[nodiscard]]
        constexpr window_handle_t viewing_window() const noexcept
        {
            return D_OS_WINDOWS_OR(viewing_window_, window());
        }

        [[nodiscard]]
        bool update_viewport() noexcept
        {
            if (const auto new_viewport = ui::sizes(viewing_window()); new_viewport.has_positive_mark()) [[likely]]
            {
                const auto is_new_viewport = new_viewport != viewport_;
                viewport_ = new_viewport;
                return is_new_viewport;
            }

            D_ASSERT(!"invalid viewport");
            return false;
        }

        [[nodiscard]]
        constexpr window_handle_t window() const noexcept
        {
            return window_;
        }

        [[nodiscard]]
        constexpr const_module_handle_t module() const noexcept
        {
            return module_;
        }

        [[nodiscard]]
        constexpr pxsizes viewport() const noexcept
        {
            return viewport_;
        }

        constexpr operator window_handle_t() const noexcept
        {
            return window();
        }

        constexpr operator const_module_handle_t() const noexcept
        {
            return module();
        }

        constexpr explicit operator bool() const noexcept
        {
            return viewport_.has_positive_mark();
        }

    private:
        pxsizes viewport_{ ui::no_sizes };
        module_handle_t module_{ nullptr };
        ui::window window_{};
        D_NO_UNIQUE_ADDRESS viewing_subwindow viewing_window_{};
    };

    static_assert(std::is_same_v<ui::event_binder_type_t<ui_owner>, const ui_owner::event_binder_type>);


    struct ui_parameters : ui::window_parameters
    {
        ui::show_command command_show{ ui::show_command::maximazed };
    };

    template<class Builder, class Params>
    class ui_gatherer : public ui::window_gatherer<Builder, Params>
    {
        using base_type = ui::window_gatherer<Builder, Params>;

    public:
        static_assert(std::disjunction_v<std::is_same<ui_parameters, Params>, std::is_base_of<ui_parameters, Params>>);

        constexpr Builder& command_show(ui::show_command command_show) noexcept
        {
            _params().command_show = command_show;
            return _builder();
        }

    protected:
        using base_type::_params;
        using base_type::_builder;
    };

    [[nodiscard]]
    inline ui_owner create_ui(const ui_parameters& params) noexcept
    {
        pxsizes viewport_sizes{ ui::no_sizes };
        auto temp_main_window = ui::create_window(params);
        viewing_subwindow temp_viewing_window{};

        if (temp_main_window) [[likely]]
        {
#ifdef D_OS_WINDOWS
            if (const auto viewing_sizes = ui::desktop_sizes(); viewing_sizes.has_positive_mark()) [[likely]]
            {
                {
                    auto viewing_window_params = params;
                    viewing_window_params.parent = temp_main_window;
                    viewing_window_params.geometry.position = { 0_npx, 0_npx };
                    viewing_window_params.geometry.sizes = viewing_sizes;
                    temp_viewing_window = ui::create_window(viewing_window_params);
                }

                if (temp_viewing_window) [[likely]]
                {
                    viewport_sizes = viewing_sizes;
                    D_ASSERT(viewport_sizes == sizes(temp_viewing_window));
                }
            }

#else
            viewport_sizes = sizes(temp_main_window);

#endif

            if (viewport_sizes.has_positive_mark()) [[likely]]
            {
                ui::show(temp_main_window, params.command_show);
            }
            else
            {
                temp_main_window = {};
            }
        }

            return
        {
            viewport_sizes,
            params.cached_type.r().module,
            std::move(temp_main_window),
            std::move(temp_viewing_window),
        };
    }
}
