#include "chartfigures.h"
#include "vectorlinechartfigure.h"

ChartFigures::~ChartFigures() noexcept = default;

void ChartFigures::add(std::vector<QPointF> points) noexcept
{
    add(std::move(points), QColor(0, 0, 255));
}

void ChartFigures::add(std::vector<QPointF> points, QPen pen) noexcept
{
    figures_.push_back(std::make_shared<VectorLineChartFigure>(points, pen));
}

Rect ChartFigures::calculateRect() const noexcept
{
    constexpr auto REAL_MAX = std::numeric_limits<qreal>::max();
    constexpr auto REAL_MIN = std::numeric_limits<qreal>::min();

    Rect rect
    {
        { REAL_MAX, REAL_MAX },
        { REAL_MIN, REAL_MIN }
    };

    for (const auto& figure : figures_)
    {
        rect = figure->calculateRect(rect);
    }

    return rect;
}

size_t ChartFigures::bufferSize() const noexcept
{
    size_t size = 0;
    for (const auto& figure : figures_)
    {
        size = std::max(size, figure->size());
    }
    return size;
}

void ChartFigures::draw(QPainter& context, span<QPointF> buffer, Transformation transform) const noexcept
{
    for (const auto& figure : figures_)
    {
        figure->draw(context, buffer, transform);
    }
}
