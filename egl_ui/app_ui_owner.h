#pragma once

#include <core/null.h>

#include <ui/window.h>
#include <ui/event_loop.h>

#include <egl_ui/fwd.h>


namespace egl_ui
{
#if defined(D_OS_WINDOWS)
    namespace private_detail_app_ui_owner
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

#endif


    struct app_ui_owner : ui::app_owner
    {
#if defined(D_OS_WINDOWS)
        struct event_binder_type
        {
            constexpr explicit event_binder_type(const app_ui_owner& egl) noexcept
                : app_event_source_{ egl.app_wnd }
                , input_event_source_{ egl.render_wnd }
            {}

            D_DISABLE_COPYMOVE_CA(event_binder_type);

            template<class EventTarget>
            [[nodiscard]] std::array<ui::event_processor, 2u> bind(EventTarget& target) const noexcept
            {
                using private_detail_app_ui_owner::event_match_one;

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
            const ui::window_handle_t app_event_source_;
            const ui::window_handle_t input_event_source_;
        };

        ui::window app_wnd;
        ui::window render_wnd;
#endif

        viewport_size2d viewport{ no_viewport };

        constexpr explicit operator bool() const noexcept
        {
            return !!viewport;
        }

        constexpr operator viewport_size2d () const noexcept
        {
            return viewport;
        }

        constexpr operator ui::viewport_event () const noexcept
        {
            return {};
        }
    };

    D_ONLY_OS_WINDOWS(static_assert(std::is_same_v<ui::event_binder_type_t<app_ui_owner>, const app_ui_owner::event_binder_type>));

    [[nodiscard]]
    D_CONDITIONAL_OS_WINDOWS(constexpr, inline) ui::window_handle_t app_window_handle(const app_ui_owner& ui) noexcept
    {
        return D_CONDITIONAL_OS_WINDOWS(ui.app_wnd, ui::app_window_handle(ui.app_module));
    }

    [[nodiscard]]
    D_CONDITIONAL_OS_WINDOWS(constexpr, inline) ui::window_handle_t render_window_handle(const app_ui_owner& ui) noexcept
    {
        return D_CONDITIONAL_OS_WINDOWS(ui.render_wnd, app_window_handle(ui));
    }

    [[nodiscard]]
    inline viewport_size2d app_ui_viewport_request(const app_ui_owner& ui) noexcept
    {
        const auto window = render_window_handle(ui);
        return (window) ? viewport_size2d{ ui::sizes(window) } : no_viewport;
    }

#ifdef D_OS_WINDOWS
    struct app_ui_parameters : ui::window_parameters
    {
        using view_type = const app_ui_parameters&;

        ui::show_command command_show{ ui::show_command::maximazed };
    };

#else
    using app_ui_parameters = ui::window_parameters;

#endif

    template<class Builder, class Params>
    class app_ui_gatherer : public ui::window_gatherer<Builder, Params>
    {
        using base_type = ui::window_gatherer<Builder, Params>;

    public:
        static_assert(std::disjunction_v<std::is_same<app_ui_parameters, Params>, std::is_base_of<app_ui_parameters, Params>>);

#ifdef D_OS_WINDOWS
        constexpr Builder& command_show(ui::show_command command_show) noexcept
        {
            _params().command_show = command_show;
            return _builder();
        }
#endif

    protected:
        using base_type::_params;
        using base_type::_builder;
    };

    [[nodiscard]]
    inline app_ui_owner ui_startup_request(view_t<app_ui_parameters> params) noexcept
    {
        D_ONLY_OS_WINDOWS(prepare(params));

        app_ui_owner result{ ui::app_startup_request(ui::mutable_app_module_handle(params)) };
        if (static_cast<const ui::app_owner&>(result)) [[likely]]
        {
#ifdef D_OS_WINDOWS
            constexpr pxsize2d invalid_sizes{ 0_npx, 0_npx };
            static_assert(!ui::window_sizes_is_valid(invalid_sizes));

            pxsize2d render_window_sizes{ invalid_sizes };

            if (params.cached_type) [[likely]]
            {
                result.app_wnd = ui::create_window(params);
            }

                if (result.app_wnd) [[likely]]
                {
                    render_window_sizes = ui::desktop_sizes();
                }

                    if (ui::window_sizes_is_valid(render_window_sizes)) [[likely]]
                    {
                        app_ui_parameters rendrer_wnd_params{ params };
                        rendrer_wnd_params.parent = result.app_wnd;
                        rendrer_wnd_params.geometry.position = { 0_npx, 0_npx };
                        rendrer_wnd_params.geometry.sizes = render_window_sizes;
                        result.render_wnd = ui::create_window(rendrer_wnd_params);
                    }

#endif

                    result.viewport = app_ui_viewport_request(result);

#ifdef D_OS_WINDOWS
                    if (result.viewport)
                    {
                        D_ASSERT(render_window_sizes == result.viewport);
                        ui::show(result.app_wnd, params.command_show);
                    }
#endif
        }

        return result;
    }
}

using egl_ui::app_ui_viewport_request;