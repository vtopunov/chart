#pragma once

#include <core/null.h>

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

    using main_window_t = ui::window;
    using render_window_t = D_CONDITIONAL_OS_WINDOWS(ui::window, dummy);

    class ui_owner
    {
    public:
        struct event_binder_type
        {
            constexpr explicit event_binder_type(const ui_owner& ui) noexcept
                : app_event_source_{ ui.window() }
                , input_event_source_{ ui.render_window() }
            {}

            D_DISABLE_COPYMOVE_CA(event_binder_type);

#ifdef D_OS_WINDOWS
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
#endif

        private:
            const window_handle_t app_event_source_;
            const window_handle_t input_event_source_;
        };

        constexpr ui_owner() noexcept = default;

        constexpr ui_owner
        (
            pxsize2d viewport,
            module_handle_t module,
            main_window_t&& window,
            render_window_t&& render_window
        ) noexcept
            : viewport_{ viewport }
            , module_{ module }
            , window_{ std::move(window) }
            , render_window_{ std::move(render_window) }
        {}

        [[nodiscard]]
        constexpr window_handle_t render_window() const noexcept
        {
            return D_CONDITIONAL_OS_WINDOWS(render_window_, window());
        }

        [[nodiscard]]
        bool update_viewport() noexcept
        {
            const auto new_viewport = ui::sizes(render_window());
            D_ASSERT(ui::window_sizes_is_valid(new_viewport));
            const auto ok = new_viewport != viewport_;
            viewport_ = new_viewport;
            return ok;
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
        constexpr pxsize2d viewport() const noexcept
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
            return ui::window_sizes_is_valid(viewport_);
        }

    private:
        pxsize2d viewport_{ ui::no_window_sizes };
        module_handle_t module_{ nullptr };
        main_window_t window_{};
        D_NO_UNIQUE_ADDRESS render_window_t render_window_{};
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
        pxsize2d viewport_sizes{ ui::no_window_sizes };
        auto temp_main_window = ui::create_window(params);
        render_window_t temp_render_window{};

        if (temp_main_window) [[likely]]
        {
#ifdef D_OS_WINDOWS
            if (const auto render_sizes = ui::desktop_sizes(); ui::window_sizes_is_valid(render_sizes)) [[likely]]
            {
                {
                    auto render_window_params = params;
                    render_window_params.parent = temp_main_window;
                    render_window_params.geometry.position = { 0_npx, 0_npx };
                    render_window_params.geometry.sizes = render_sizes;
                    temp_render_window = ui::create_window(render_window_params);
                }

                if (temp_render_window) [[likely]]
                {
                    viewport_sizes = render_sizes;
                    D_ASSERT(viewport_sizes == sizes(temp_render_window));
                }
            }

#else
            viewport_sizes = sizes(temp_main_window);

#endif

            if (ui::window_sizes_is_valid(viewport_sizes)) [[likely]]
            {
                ui::show(temp_main_window, params.command_show);
            }
            else
            {
                temp_main_window = null_v<main_window_t>;
            }
        }

            return
        {
            viewport_sizes,
            params.cached_type.r().module,
            std::move(temp_main_window),
            std::move(temp_render_window),
        };
    }
}

template<class T>
[[nodiscard]] constexpr auto viewport(const T& source) noexcept -> decltype(source.viewport())
{
    return source.viewport();
}