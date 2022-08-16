#pragma once

#include <px/fwd.h>

namespace egl_ui
{
    struct viewport_rectangle
    {
        px::size2d sizes;

        constexpr explicit operator bool() const noexcept
        {
            static_assert(std::is_unsigned_v<decltype(sizes.width())>);
            static_assert(std::is_unsigned_v<decltype(sizes.height())>);
            return !!sizes.width() && !!sizes.height();
        }

        [[nodiscard]]
        constexpr pxside_t width() const noexcept
        {
            return sizes.width();
        }

        [[nodiscard]]
        constexpr pxside_t height() const noexcept
        {
            return sizes.height();
        }
    };

    [[nodiscard]]
    constexpr px::size2d sizes(const viewport_rectangle& v) noexcept
    {
        return v.sizes;
    }

    [[nodiscard]]
    constexpr pxside_t width(const viewport_rectangle& v) noexcept
    {
        return v.width();
    }

    [[nodiscard]]
    constexpr pxside_t height(const viewport_rectangle& v) noexcept
    {
        return v.height();
    }
}

using egl_ui::viewport_rectangle;
using egl_ui::sizes;
using egl_ui::width;
using egl_ui::height;
