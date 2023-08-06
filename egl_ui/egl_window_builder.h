#pragma once

#include <os/fwd.h>

#include <ui/window_constants.h>

#include <egl_ui/ui_window_parametrs.h>
#include <egl_ui/egl_window.h>


namespace egl_ui
{
    struct egl_window_parameters
    {
        ui_window_parametrs ui_params;
        gl::rgba_colorf_t background{ gl::colors::white_f };

#ifdef D_OS_WINDOWS
        ui::show_command command_show{ ui::show_command::maximazed };
#endif
    };

    [[nodiscard]]
    egl_window create_egl_window(const egl_window_parameters& params) noexcept;

    [[nodiscard]]
    inline egl_window create_egl_window(os::module_handle_t module) noexcept
    {
        return create_egl_window(egl_window_parameters{ .ui_params{.module{ module } }});
    }

    template<class Builder>
    class egl_window_gatherer
    {
    public:
        constexpr Builder& module(os::module_handle_t module) noexcept
        {
            params_.ui_params.module = module;
            return _builder();
        }

        constexpr Builder& background(gl::rgba_colorf_t color) noexcept
        {
            params_.background = color;
            return _builder();
        }

#ifdef D_OS_WINDOWS
        constexpr Builder& command_show(ui::show_command command) noexcept
        {
            params_.command_show = command;
            return _builder();
        }

        constexpr Builder& position(pxpoint2d position) noexcept
        {
            params_.ui_params.geometry.position = position;
            return _builder();
        }

        constexpr Builder& position(pxside_t x, pxside_t y) noexcept
        {
            return position(pxpoint2d{ x, y });
        }

        constexpr Builder& sizes(pxsize2d sizes) noexcept
        {
            params_.ui_params.geometry.sizes = sizes;
            return _builder();
        }

        constexpr Builder& sizes(pxside_t width, pxside_t height) noexcept
        {
            return sizes(pxsize2d{ width, height });
        }

        constexpr Builder& geometry(const pxrectangle& rc) noexcept
        {
            params_.ui_params.geometry = rc;
            return _builder();
        }
#endif

    protected:
        [[nodiscard]]
        constexpr const egl_window_parameters& params() const noexcept
        {
            return params_;
        }

    private:
        [[nodiscard]]
        constexpr Builder& _builder() noexcept
        {
            static_assert(std::is_base_of_v<egl_window_gatherer, Builder>);
            return static_cast<Builder&>(*this);
        }

    private:
        egl_window_parameters params_;
    };

    class egl_window_builder : public egl_window_gatherer<egl_window_builder>
    {
    public:
        [[nodiscard]]
        egl_window build() const noexcept
        {
            return create_egl_window(params());
        }
    };

}

using egl_ui::egl_window_builder;
using egl_ui::create_egl_window;
