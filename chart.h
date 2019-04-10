#pragma once

#include <QWidget>

#include "rect.h"
#include "chartfigures.h"
#include "chartfiguresrect.h"
#include "chartwidgetrect.h"
#include "chartbuffer.h"

using ChartBufferPtr = std::shared_ptr<ChartBuffer>;

class Chart : public QWidget
{
    Q_OBJECT
public:
    QPen axisframepen;
    QBrush background;
    QMarginsF margins;
    ChartFigures figures;

public:
    Chart(QWidget* parent = nullptr) noexcept;

    void clear() noexcept;
    bool updateRects() noexcept;
    bool isValidRects() const noexcept;

protected:
    void paintEvent(QPaintEvent*) noexcept override;
    void mouseDoubleClickEvent(QMouseEvent*) noexcept override;
    void wheelEvent(QWheelEvent*) noexcept override;
    void mouseMoveEvent(QMouseEvent*) noexcept override;
    void mousePressEvent(QMouseEvent*) noexcept override;
    void mouseReleaseEvent(QMouseEvent*) noexcept override;

private:
    ChartWidgetRect widgetRect_;
    ChartFiguresRect figuresRect_;
    QPointF mousePos_;
    ChartBufferPtr buffer_;
};

