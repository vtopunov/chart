#pragma once

#include <vector>
#include <memory>

#include <qpen.h>

#include "rect.h"
#include "transformation.h"

class IChartFigure;

class ChartFigures
{
public:
    ChartFigures() noexcept = default;

    ~ChartFigures() noexcept;

    void add(std::vector<QPointF> points) noexcept;

    void add(std::vector<QPointF> points, QPen pen) noexcept;

    Rect calculateRect() const noexcept;

    size_t bufferSize() const noexcept;

    void draw(QPainter& context, span<QPointF> buffer, Transformation transform) const noexcept;

private:
    std::vector<std::shared_ptr<IChartFigure>> figures_;
};
