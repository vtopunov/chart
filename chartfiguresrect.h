#pragma once

#include "transformation.h"

class ChartFigures;

class ChartFiguresRect
{
public:
    friend class ChartWidgetRect;

    bool zoom(const QPointF& zoom) noexcept;

    bool move(const QPointF& move) noexcept;

    bool isValid() const noexcept;

    explicit operator bool() const noexcept
    {
        return isValid();
    }

    Transformation to(const ChartWidgetRect&) const noexcept;

    bool update(const ChartFigures& figures) noexcept;

private:
    bool set(const Rect& rect) noexcept;

private:
    Rect rect_;
};