#pragma once

#include "transformation.h"

class QWidget;
class QMarginsF;

class ChartWidgetRect
{
public:
    friend class ChartFiguresRect;

    QRectF frame(qreal frame) const noexcept;

    QRectF clipRect() const noexcept;

    bool in(const QPointF& point) const noexcept;

    bool isValid() const noexcept;

    explicit operator bool() const noexcept
    {
        return isValid();
    }

    Transformation to(const ChartFiguresRect&) const noexcept;

    bool update(const QWidget& widget, const QMarginsF& margins) noexcept;

private:
    bool set(const Rect& rect) noexcept;

private:
    Rect rect_;
};