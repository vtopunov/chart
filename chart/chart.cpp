#include <qpainter.h>
#include <qevent.h>

#include <platform/painter.h>
#include <platform/platform_cast.h>

#include "chart.h"

namespace
{
    bool updateCoordinateRect(coordiante_rect<coordiante_system::windows>& rect, const QWidget& widget, QMarginsF margins) noexcept
    {
        return rect.set( platform_cast<rect_t>( QRectF{ widget.rect() } - margins ) );
    }

    bool updateCoordinateRect(coordiante_rect<coordiante_system::math>& rect, const chart_figures& figures) noexcept
    {
        if (!rect)
        {
            return rect.set(figures.calculate_rect());
        }

        return true;
    }
}

Chart::Chart(QWidget* parent) noexcept
    : QWidget(parent)
    , buffer{ BufferPtr(), &buffer::default_instance() }
{
    clear();
}

void Chart::paintEvent(QPaintEvent*) noexcept
{
    if (updateCooridinate())
    {
        QPainter q_painter{ this };
        painter context{ &q_painter };

        context.pen(axisframepen);
        context.brush(background);
        context.draw_rect(windows.rect().with_frame(1.0));

        context.clip(windows.rect());
        context.antialiasing(true);

        const auto to_windows_coordinate = math.to(windows);
        const auto points = buffer->get<point_t>(figures.buffer_size());
        figures.draw(context, points, to_windows_coordinate);
    }
}


void Chart::clear() noexcept
{
    figures = {};
    axisframepen = colors::black;
    background = colors::white;
    mousePos_ = {};
    math = {};
    setMouseTracking(false);
}

bool Chart::updateCooridinate() noexcept
{
    return updateCoordinateRect(windows, *this, margins) && updateCoordinateRect(math, figures);
}

void Chart::mouseDoubleClickEvent(QMouseEvent * e) noexcept
{
    D_ASSERT(e);

    if (!math)
    {
        return;
    }

    if (windows.rect().includes(platform_cast<point_t>(e->localPos())))
    {
        math = {};
        repaint();
    }
}

void Chart::mouseMoveEvent(QMouseEvent * e) noexcept
{
    D_ASSERT(e);

    if (!math)
    {
        return;
    }

    if (e->buttons() != Qt::LeftButton)
    {
        mousePos_ = {};
        return;
    }

    const auto& new_pos = e->localPos();
    const auto old_pos = mousePos_;
    mousePos_ = new_pos;

    if (old_pos.isNull())
    {
        return;
    }

    const auto transfom = windows.to( math );
    const auto math_pos0 = transfom( platform_cast<point_t>( new_pos ) ); 
    const auto math_pos1 = transfom( platform_cast<point_t>( old_pos ) );
    if (!math.move(math_pos1 - math_pos0))
    {
        return;
    }

    repaint();
}

void Chart::mousePressEvent(QMouseEvent*) noexcept
{
     setMouseTracking(!!math);
}

void Chart::mouseReleaseEvent(QMouseEvent*) noexcept
{
    mousePos_ = {};
}

void Chart::wheelEvent(QWheelEvent * e) noexcept
{
    D_ASSERT(e);

    if (!math)
    {
        return;
    }

    constexpr int min_zDelta = 120;
    constexpr double zoom_factor = 1.1;

    const auto nzoom = static_cast<double>(e->delta()) / min_zDelta;
    const auto zoom = pow(zoom_factor, nzoom);
    if (!math.zoom(point_t::fill(zoom)))
    {
        return;
    }

    repaint();
}


