#include "chartwidgetrect.h"
#include "chartfiguresrect.h"

#include <qwidget.h>

QRectF ChartWidgetRect::frame(qreal frame) const noexcept
{
    return rect_.frame(frame).toQRectF();
}

QRectF ChartWidgetRect::clipRect() const noexcept
{
    return rect_.toQRectF();
}

bool ChartWidgetRect::in(const QPointF& point) const noexcept
{
    return rect_.in(point);
}

Transformation ChartWidgetRect::to(const ChartFiguresRect& to) const noexcept
{
    return ::transformation(rect_, to.rect_.inverse<YAxis>());
}

bool ChartWidgetRect::update(const QWidget& widget, const QMarginsF& margins) noexcept
{
    return set(Rect::fromQRectF(QRectF(widget.geometry()) - margins));
}