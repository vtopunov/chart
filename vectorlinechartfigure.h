#pragma once

#include <vector>
#include <qpen.h>

#include "ichartfigure.h"

class VectorLineChartFigure final : public IChartFigure
{
public:
    VectorLineChartFigure(std::vector<QPointF> points, QPen pen) noexcept
        : points_(std::move(points))
        , pen_(std::move(pen))
    {}

    size_t size() const noexcept final;

    Rect calculateRect(Rect rect) const noexcept final;

    void draw(QPainter& context, span<QPointF> buffer, Transformation) const noexcept final;

private:
    std::vector<QPointF> points_;
    QPen pen_;
};


