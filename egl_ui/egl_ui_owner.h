#pragma once

#include <egl_ui/ui_owner.h>
#include <egl_ui/egl_context.h>


namespace egl_ui
{
    class egl_ui_owner : public ui_owner
    {
    public:
        constexpr egl_ui_owner() noexcept = default;

        constexpr egl_ui_owner(ui_owner&& ui, egl_context&& egl) noexcept
            : ui_owner{ std::move(ui) }
            , egl_{ std::move(egl) }
        {}

        constexpr explicit operator bool() const noexcept
        {
            return !!egl_;
        }

        constexpr operator display_surface () const noexcept
        {
            return egl_;
        }

    private:
        egl_context egl_;
    };

    using egl_ui_parameters = ui_parameters;

    [[nodiscard]]
    inline egl_ui_owner create_egl_ui(const egl_ui_parameters& params) noexcept
    {
        auto ui = create_ui(params);
        egl_context egl{};

        if (ui) [[likely]]
        {
            egl = create_egl_context(ui.viewing_window());
        }

        return 
        {
            std::move(ui),
            std::move(egl)
        };
    }

    template<class Builder, class Params>
    using egl_ui_gatherer = ui_gatherer<Builder, Params>;

    struct egl_ui_builder : egl_ui_gatherer<egl_ui_builder, egl_ui_parameters>
    {
        [[nodiscard]]
        egl_ui_owner build() const noexcept
        {
            return create_egl_ui(_c_params());
        }
    };

    [[nodiscard]]
    inline egl_ui_owner create_egl_ui(ui::module_handle_t module) noexcept
    {
        return egl_ui_builder{}.module(module).build();
    }
}

using egl_ui::egl_ui_owner;
using egl_ui::egl_ui_builder;
using egl_ui::egl_ui_parameters;
using egl_ui::create_egl_ui;