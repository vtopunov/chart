#include "vectorlinechartfigure.h"
#include <qpainter.h>

size_t VectorLineChartFigure::size() const noexcept
{
    return points_.size();
}

Rect VectorLineChartFigure::calculateRect(Rect rect) const noexcept
{
	return rect.include(points_);
}

void VectorLineChartFigure::draw(QPainter& context, span<QPointF> buffer, Transformation transform) const noexcept
{
	assert(points_.size() <= buffer.size());

	size_t index = 0;
	for (const auto& point : points_)
	{
		buffer[index] = transform(point);
		++index;
	}

	context.setPen(pen_);
	context.drawPolyline(buffer.data(), narrow_cast<int>(index));
}
