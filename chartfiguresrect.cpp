#include "chartfiguresrect.h"
#include "chartwidgetrect.h"
#include "chartfigures.h"

bool ChartFiguresRect::zoom(const QPointF& zoom) noexcept
{
    return set(rect_.zoom(zoom));
}

bool ChartFiguresRect::move(const QPointF& move) noexcept
{
    return set(rect_.move(move));
}

Transformation ChartFiguresRect::to(const ChartWidgetRect& to) const noexcept
{
    return ::transformation(rect_.inverse<YAxis>(), to.rect_);
}

bool ChartFiguresRect::update(const ChartFigures& figures) noexcept
{
    if (!isValid())
    {
        return set(figures.calculateRect());
    }

    return true;
}
