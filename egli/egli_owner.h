#pragma once

#include <egli/ui_owner.h>
#include <egli/egl_context.h>


namespace egli
{
    class egli_owner : public ui_owner
    {
    public:
        constexpr egli_owner() noexcept = default;

        constexpr egli_owner(ui_owner&& ui, egl_context&& egl) noexcept
            : ui_owner{ std::move(ui) }
            , egl_{ std::move(egl) }
        {}

        constexpr explicit operator bool() const noexcept
        {
            const auto result = !!egl_;
            D_ASSERT(result == !!static_cast<const ui_owner&>(*this));
            return result;
        }

        constexpr operator display_surface () const noexcept
        {
            return egl_;
        }

    private:
        egl_context egl_;
    };

    using egli_parameters = ui_parameters;

    [[nodiscard]]
    inline egli_owner create_egli(const egli_parameters& params) noexcept
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
    using egli_gatherer = ui_gatherer<Builder, Params>;

    struct egli_builder : egli_gatherer<egli_builder, egli_parameters>
    {
        [[nodiscard]]
        egli_owner build() const noexcept
        {
            return create_egli(_c_params());
        }
    };
}

using egli::egli_owner;
using egli::egli_builder;
using egli::egli_parameters;
using egli::create_egli;