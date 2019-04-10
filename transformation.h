#pragma once

#include "rect.h"

struct TransformationAxis
{
    qreal scaling_, offset_;

    constexpr qreal operator () (qreal value) const noexcept
    {
        return scaling_ * value + offset_;
    }

    constexpr TransformationAxis withOffset(qreal offset) const noexcept
    {
        return { scaling_, offset };
    }

    constexpr TransformationAxis withScaling(qreal scaling) const noexcept
    {
        return { scaling, offset_ };
    }
};

template<AxisType axis>
constexpr TransformationAxis transformationAxis(const Rect& from, const Rect& to) noexcept
{
    return
    {
        to.length<axis>() / from.length<axis>(),
        (get<axis>(to.p0) * get<axis>(from.p1) - get<axis>(to.p1) * get<axis>(from.p0)) / from.length<axis>()
    };
}

struct Transformation
{
    TransformationAxis x, y;

    constexpr QPointF operator () (const QPointF& point) const noexcept
    {
        return { x(point.x()), y(point.y()) };
    }

    constexpr Transformation withOffset(const QPointF& point) const noexcept
    {
        return { x.withOffset(point.x()), y.withOffset(point.y()) };
    }

    constexpr Transformation withScaling(const QPointF& point) const noexcept
    {
        return { x.withScaling(point.x()), y.withScaling(point.y()) };
    }
};

constexpr Transformation transformation(const Rect & from, const Rect & to) noexcept
{
    return
    {
        transformationAxis<XAxis>(from, to),
        transformationAxis<YAxis>(from, to)
    };
}

