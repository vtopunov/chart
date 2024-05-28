#pragma once

#include <core/resource.h>
#include <core/color.h>

#include <ui/fwd.h>


namespace ui
{
    enum class stock_brush
    {
        white,
        light_gray,
        gray,
        dark_gray,
        black,
        null
    };

#ifdef D_OS_WINDOWS
    struct gdi_object_deleter
    {
        void operator () (gdi_object_handle_t o) const noexcept;
    };

    using unique_brush = unique_resource<brush_handle_t, gdi_object_deleter>;

    [[nodiscard]] unique_brush create_brush(rgba_color color) noexcept;

    [[nodiscard]] brush_handle_t stock(stock_brush brush) noexcept;

#else
    struct dummy_unique_brush {};
    struct dummy_brush_handle {};
    struct dummy_const_brush_handle {};

    using unique_brush = dummy_unique_brush;
    using brush_handle_t = dummy_brush_handle;
    using const_brush_handle_t = dummy_const_brush_handle;

    [[nodiscard]]
    constexpr unique_brush create_brush(rgba_color) noexcept
    {
        return {};
    }

    [[nodiscard]]
    constexpr brush_handle_t stock(stock_brush brush) noexcept
    {
        return {};
    }

#endif
}