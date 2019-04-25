#include "chartfigures.h"
#include "vectorlinechartfigure.h"

#include <thread>

chart_figures::~chart_figures() noexcept = default;

void chart_figures::add(std::vector<point_t> points, color pen) noexcept
{
    figures.push_back(std::make_shared<vector_line_chart_figure>(points, pen));
}

rect_t chart_figures::calculate_rect() const noexcept
{
    rect_t rect
    {
        point_t::fill(max_v<real_t>),
        point_t::fill(lowest_v<real_t>)
    };

    for (const auto& figure : figures)
    {
        rect = figure->calculate_rect(rect);
    }

    return rect;
}

size_t chart_figures::buffer_size() const noexcept
{
    size_t size = 0;
    for (const auto& figure : figures)
    {
        size = std::max(size, figure->size());
    }
    return size;
}

void chart_figures::draw(painter& context, span<point_t> buffer, coordinate_transformation to_windows_coordinate) const noexcept
{
    for (const auto& figure : figures)
    {
        figure->draw(context, buffer, to_windows_coordinate);
    }
}
