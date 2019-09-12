#pragma once

#include "core/coordinate.h"
#include "platform/painter.h"


class IChartFigure
{
public:
    virtual ~IChartFigure() noexcept = default;

    virtual rect_t calculate_rect(rect_t rect) const noexcept = 0;

    virtual size_t size() const noexcept = 0;

    virtual void draw(painter& context, span<point_t> buffer, coordinate_transformation to_windows_coordinate) const noexcept = 0;
};
