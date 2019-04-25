#pragma once

#include <QWidget>

#include <core/coordinate.h>
#include <core/buffer.h>

#include "chartfigures.h"

using BufferPtr = std::shared_ptr<buffer>;

class Chart final : public QWidget
{
public:
    color axisframepen;
    color background;
    QMarginsF margins;
    chart_figures figures;
    BufferPtr buffer;

public:
    Chart(QWidget* parent = nullptr) noexcept;

    void clear() noexcept;
    bool updateCooridinate() noexcept;

private:
    void paintEvent(QPaintEvent*) noexcept final;
    void mouseDoubleClickEvent(QMouseEvent*) noexcept final;
    void wheelEvent(QWheelEvent*) noexcept final;
    void mouseMoveEvent(QMouseEvent*) noexcept final;
    void mousePressEvent(QMouseEvent*) noexcept final;
    void mouseReleaseEvent(QMouseEvent*) noexcept final;

private:
    coordiante_rect<coordiante_system::windows> windows;
    coordiante_rect<coordiante_system::math> math;
    QPointF mousePos_;
};

