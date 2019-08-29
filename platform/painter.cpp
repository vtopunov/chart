#include "painter.h"
#include "platform_cast.h"

#pragma warning(push, 0)
#include <qpainter.h>
#include <qpaintengine.h>
#pragma warning(pop)

namespace
{
    constexpr QPainter* qpainter( void* context ) noexcept
    {
        return static_cast<QPainter*>( context );
    }
}

void painter::pen(color color) noexcept
{
    qpainter( context )->setPen(platform_cast<QColor>(color));
}

void painter::brush(color color) noexcept
{
    qpainter( context )->setBrush(platform_cast<QColor>( color ));
}

void painter::draw_rect(rect_t rect) noexcept
{
    qpainter( context )->drawRect(platform_cast<QRectF>( rect ));
}

void painter::clip(rect_t rect) noexcept
{
    qpainter( context )->setClipRect(platform_cast<QRectF>( rect ));
    qpainter( context )->setClipping(true);
}

void painter::antialiasing(bool enable) noexcept
{
    qpainter( context )->setRenderHint(QPainter::RenderHint::Antialiasing, enable);
}

void painter::draw_polyline(span<const point_t> polyline) noexcept
{
    const auto qpoints = platform_cast<span<const QPointF>>( polyline );
    qpainter( context )->drawPolyline(qpoints.data(), narrow_cast<int>( qpoints.size() ));
}
