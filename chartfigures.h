#pragma once

#include <vector>
#include <memory>

#include <core/span.h>
#include <core/rect.h>
#include <core/color.h>
#include <core/coordinate.h>

class IChartFigure;
class painter;

constexpr color default_figure_color = colors::blue;

class chart_figures
{
public:
    chart_figures() noexcept = default;

    ~chart_figures() noexcept;

    void add(std::vector<point_t> points, color pen) noexcept;

    void add(span<const point_t> points, color pen) noexcept
    {
        add(std::vector<point_t>{ points.begin(), points.end() }, pen);
    }

    template<class Container, class = decltype( std::data(std::declval<Container>()) ), class = decltype( std::size(std::declval<Container>()) )>
    void add(Container&& contaniner) noexcept
    {
        add(std::forward<Container>(contaniner), default_figure_color);
    }

    rect_t calculate_rect() const noexcept;

    size_t buffer_size() const noexcept;

    void draw(painter& context, span<point_t> buffer, coordinate_transformation toWindowsCoordinate) const noexcept;

private:
    std::vector<std::shared_ptr<IChartFigure>> figures;
};
