#include "chart.h"
#include "transformation.h"

#include <qpainter.h>
#include <qevent.h>


Chart::Chart(QWidget* parent) noexcept
    : QWidget(parent)
    , buffer_{ ChartBufferPtr(), &ChartBuffer::default() }
{
    clear();
}

void Chart::paintEvent(QPaintEvent*) noexcept
{
    if (updateRects())
    {
        QPainter context{ this };

        context.setPen(axisframepen);
        context.setBrush(background);

        context.drawRect(widgetRect_.frame(axisframepen.widthF() + 1.0));

        context.setClipRect(widgetRect_.clipRect());
        context.setClipping(true);
        context.setRenderHint(QPainter::Antialiasing, true);

        const auto transform = figuresRect_.to(widgetRect_);
        const auto buffer = buffer_->getPoints(figures.bufferSize());
        figures.draw(context, buffer, transform);
    }
}


void Chart::clear() noexcept
{
    figures = {};
    axisframepen = QColor(0, 0, 0);
    background = QColor(255, 255, 255, 255);
    mousePos_ = {};
    figuresRect_ = {};
    setMouseTracking(false);
}

bool Chart::updateRects() noexcept
{
    return widgetRect_.update(*this, margins) && figuresRect_.update(figures);
}

bool Chart::isValidRects() const noexcept
{
    return widgetRect_ && figuresRect_;
}

void Chart::mouseDoubleClickEvent(QMouseEvent * e) noexcept
{
    assert(e != nullptr);

    if (isValidRects() && widgetRect_.in(e->localPos()))
    {
        figuresRect_ = {};
        repaint();
    }
}

void Chart::mouseMoveEvent(QMouseEvent * e) noexcept
{
    assert(e != nullptr);

    if (!isValidRects())
    {
        return;
    }

    if (e->buttons() != Qt::LeftButton)
    {
        mousePos_ = {};
        return;
    }

    const auto& pos = e->localPos();
    const auto mousePos = mousePos_;
    mousePos_ = pos;

    if (mousePos.isNull())
    {
        return;
    }

    const auto transfom = widgetRect_.to(figuresRect_).withOffset({ 0.0, 0 });
    const auto move = transfom(mousePos) - transfom(pos);
    if (!figuresRect_.move(move))
    {
        return;
    }

    repaint();
}

void Chart::mousePressEvent(QMouseEvent*) noexcept
{
    setMouseTracking(figuresRect_.isValid());
}

void Chart::mouseReleaseEvent(QMouseEvent*) noexcept
{
    mousePos_ = {};
}

void Chart::wheelEvent(QWheelEvent * e) noexcept
{
    assert(e != nullptr);

    if (!isValidRects())
    {
        return;
    }

    constexpr int min_zDelta = 120;
    constexpr double zoomFactor = 1.1;

    const auto nzoom = static_cast<double>(e->delta()) / min_zDelta;
    const auto zoom = pow(zoomFactor, nzoom);
    if (!figuresRect_.zoom(equalAxisPoint(zoom)))
    {
        return;
    }

    repaint();
}


