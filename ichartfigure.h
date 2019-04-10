#pragma once

#include "span.h"
#include "transformation.h"
#include "rect.h"

class QPainter;

class IChartFigure
{
public:
    virtual ~IChartFigure() noexcept = default;

    virtual Rect calculateRect(Rect rect) const noexcept = 0;

    virtual size_t size() const noexcept = 0;

    virtual void draw(QPainter& context, span<QPointF> buffer, Transformation) const noexcept = 0;
};
