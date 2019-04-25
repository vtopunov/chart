#pragma once

#include <vector>

#include "ichartfigure.h"

class vector_line_chart_figure final : public IChartFigure
{
public:
    vector_line_chart_figure(std::vector<point_t> points, color pen) noexcept
        : points_(std::move(points))
        , pen_(std::move(pen))
    {}

    size_t size() const noexcept final;

    rect_t calculate_rect(rect_t rect) const noexcept final;

    void draw(painter& context, span<point_t> buffer, coordinate_transformation to_windows_coordinate) const noexcept final;

private:
    std::vector<point_t> points_;
    color pen_;
};


