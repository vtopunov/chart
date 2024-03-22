#include "brush.h"

#include <os/os.h>


namespace ui
{
    void gdi_object_deleter::operator()(gdi_object_handle_t o) const noexcept
    {
        if (o)
        {
            D_ASSERT_OR_UNUSED(DeleteObject(o));
        }
    }

    unique_brush create_brush(rgba_color_t color) noexcept
    {
        return
        {
            resource_construct,
            CreateSolidBrush(RGB(color.r, color.g, color.b))
        };
    }

    brush_handle_t stock(stock_brush brush) noexcept
    {
        static_assert(std::is_same_v<int, std::underlying_type_t<stock_brush>>);
        static_assert(WHITE_BRUSH == to_underlying(stock_brush::white));
        static_assert(LTGRAY_BRUSH == to_underlying(stock_brush::light_gray));
        static_assert(GRAY_BRUSH == to_underlying(stock_brush::gray));
        static_assert(DKGRAY_BRUSH == to_underlying(stock_brush::dark_gray));
        static_assert(BLACK_BRUSH == to_underlying(stock_brush::black));
        static_assert(NULL_BRUSH == to_underlying(stock_brush::null));

        return static_cast<HBRUSH>(GetStockObject(to_underlying(brush)));
    }
}
