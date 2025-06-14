#pragma once

#include <gl_core/draw.h>

#include <egli/egli_owner.h>

#include <widget/fwd.h>


namespace widget
{
    namespace colors
    {
        using namespace ::color_literals;
        using namespace ::colors;

        constexpr auto dialog_color = 0xf0f0f0_rgb;;
        constexpr auto dialog_color_f = to_colorf(dialog_color);
    }

    class window_content
    {
    public:
        [[nodiscard]]
        constexpr pxsizes sizes() const noexcept
        {
            return sizes_;
        }

    private:
        friend class window;

    private:
        pxsizes sizes_{ no_sizes };
    };
    
    using window_parameters = egli_parameters;

    class window : public egli_owner
    {
    public:
        constexpr window() noexcept = default;

        explicit window(const window_parameters& params) noexcept
            : egli_owner{ create_egli(params) }
        {
            if (std::as_const<egli_owner>(*this)) [[likely]]
            {
                gl::clear_color(colors::dialog_color_f);
            }
        }

        D_DISABLE_COPYMOVE_CA(window);

        constexpr dummy apply(no_overload) const noexcept
        {
            return dummy_v;
        }

        [[nodiscard]]
        constexpr window_content content() const noexcept
        {
#ifdef D_OS_WINDOWS
            return content_cache_;
#else
            window_content temp{};
            temp.sizes_ = egli_owner::viewport();
            return temp;
#endif
    }

    public:
        constexpr void content_sizes(pxsizes sizes) noexcept
        {
#ifdef D_OS_WINDOWS
            content_cache_.sizes_ = sizes;
#else
            D_UNUSED(sizes);
#endif
}

    private:
        D_ONLY_OS_WINDOWS(window_content content_cache_{});
    };


    struct window_builder : egli::egli_gatherer<window_builder, window_parameters>
    {
#ifdef D_OS_WINDOWS
        window_builder() noexcept
        {
            background(ui::create_brush(colors::dialog_color));
            D_ASSERT(background());
        }
#endif

        [[nodiscard]] window build() const noexcept
        {
            return window{ _c_params() };
        }
    };
}