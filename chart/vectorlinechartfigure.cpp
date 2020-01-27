#include "vectorlinechartfigure.h"

#include <core/coordinate.h>


size_t vector_line_chart_figure::size() const noexcept
{
    return points_.size();
}

rect_t vector_line_chart_figure::calculate_rect(rect_t rect) const noexcept
{
    for (const auto& point : points_)
    {
        rect = rect.with_inclusion(point);
    }

    return rect;
}

void vector_line_chart_figure::draw(painter& context, span<point_t> buffer, coordinate_transformation to_windows_coordinate) const noexcept
{
	D_ASSERT(points_.size() <= buffer.size());

    buffer = buffer.left(points_.size());

	size_t index = 0;
	for (const auto point : points_)
	{
		buffer[index] = to_windows_coordinate(point);
		++index;
	}

    context.pen(pen_);
    context.draw_polyline(buffer);
}
