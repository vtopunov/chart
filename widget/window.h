#pragma once

#include <gl/color.h>

#include <egl_ui/egl_ui_owner.h>

#include <widget/fwd.h>


namespace widget
{
    namespace colors
    {
        using namespace ::color_literals;

        constexpr auto dialog_color = 0xf0f0f0_rgb;
        constexpr auto gl_dialog_color_f = gl::to_colorf(dialog_color);
    }

    class window : public egl_ui_owner
    {
    public:
        constexpr window() noexcept = default;

        explicit window(view_t<egl_ui_parameters> params) noexcept
            : egl_ui_owner{ create_egl_ui(params) }
        {}

        D_DISABLE_COPYMOVE_CA(window);

        constexpr operator content_size2d () const noexcept
        {
            return { D_CONDITIONAL_OS_WINDOWS(content_sizes_cache_, as_size2d(viewport)) };
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }

    public:
        constexpr void content_sizes(pxsize2d sizes) noexcept
        {
#ifdef D_OS_WINDOWS
            content_sizes_cache_ = sizes;
#else
            D_UNUSED(sizes);
#endif
        }

    private:
        D_ONLY_OS_WINDOWS(pxsize2d content_sizes_cache_{ ui::no_window_sizes });
    };

    struct window_builder : egl_ui::egl_ui_gatherer<window_builder>
    {
#ifdef D_OS_WINDOWS
        window_builder() noexcept
        {
            background(ui::create_brush(colors::dialog_color));
            D_ASSERT(background());
        }
#endif

        [[nodiscard]]
        window build() const noexcept
        {
#ifdef D_OS_WINDOWS
            if (background()) [[likely]]
            {
                return window{ _c_params() };
            }

            return {};
#else
            return window{ _c_params() };
#endif
        }
    };
}