#pragma once

#include <type_traits>
#include <utility>
#include <algorithm>

#include <qpoint.h>
#include <qrect.h>

#include "span.h"

enum AxisType
{
    XAxis = 1,
    YAxis = 1 << 1,
    XYAxis = XAxis | YAxis
};

template <AxisType axis>
using axis_constant = std::integral_constant<AxisType, axis>;

using x_axis_type = axis_constant<XAxis>;
using y_axis_type = axis_constant<YAxis>;
using xy_axis_type = axis_constant<XYAxis>;

constexpr qreal get(const QPointF& point, x_axis_type) noexcept
{
    return point.x();
}

constexpr qreal get(const QPointF& point, y_axis_type) noexcept
{
    return point.y();
}

template<AxisType axis>
constexpr qreal get(const QPointF& point) noexcept
{
    return get(point, axis_constant<axis>());
}

constexpr QPointF min(const QPointF& a, const QPointF& b) noexcept
{
    return
    {
        std::min(a.x(), b.x()),
        std::min(a.y(), b.y())
    };
}

constexpr QPointF max(const QPointF& a, const QPointF& b) noexcept
{
    return
    {
        std::max(a.x(), b.x()),
        std::max(a.y(), b.y())
    };
}

constexpr QPointF mul(const QPointF& a, const QPointF& b) noexcept
{
    return
    {
        a.x() * b.x(),
        a.y() * b.y()
    };
}

constexpr QPointF equalAxisPoint(qreal value) noexcept
{
    return { value, value };
}

struct Rect
{
    struct Range
    {
        qreal p0, p1;

        constexpr bool in(qreal value) const noexcept
        {
            return value >= p0 && value <= p1;
        }

        constexpr qreal length() const noexcept
        {
            return p1 - p0;
        }
    };

    QPointF p0, p1;

    template<AxisType axis>
    constexpr Range range() const noexcept
    {
        return { get<axis>(p0), get<axis>(p1) };
    }

    template<AxisType axis>
    constexpr qreal length() const noexcept
    {
        return range<axis>().length();
    }

    constexpr qreal width() const noexcept
    {
        return length<XAxis>();
    }

    constexpr qreal height() const noexcept
    {
        return length<YAxis>();
    }

    constexpr QPointF size() const noexcept
    {
        return p1 - p0;
    }

    constexpr QPointF mean() const noexcept
    {
        return (p1 + p0) / 2;
    }

    constexpr Rect inverse(x_axis_type) const noexcept
    {
        return
        {
            { p1.x(), p0.y() },
        { p0.x(), p1.y() },
        };
    }

    constexpr Rect inverse(y_axis_type) const noexcept
    {
        return
        {
            { p0.x(), p1.y() },
        { p1.x(), p0.y() },
        };
    }

    constexpr Rect inverse(xy_axis_type) const noexcept
    {
        return { p1, p0 };
    }

    template<AxisType axis = XYAxis>
    constexpr Rect inverse() const noexcept
    {
        return inverse(axis_constant<axis>());
    }

    constexpr Rect include(const QPointF & point) const noexcept
    {
        return { min(p0, point), max(p1, point) };
    }

    constexpr Rect include(const span<const QPointF> points) const noexcept
    {
        Rect rect{ *this };

        for (const auto& point : points)
        {
            rect = rect.include(point);
        }

        return rect;
    }

    template<AxisType axis>
    constexpr bool in(const QPointF & value, axis_constant<axis>) const noexcept
    {
        return range<axis>().in(get<axis>(value));
    }

    constexpr bool in(const QPointF & value, xy_axis_type) const noexcept
    {
        return in(value, x_axis_type()) && in(value, y_axis_type());
    }

    template<AxisType axis = XYAxis>
    constexpr bool in(const QPointF & value) const noexcept
    {
        return in(value, axis_constant<axis>());
    }

    constexpr bool in(const Rect & rect) const noexcept
    {
        return in(rect.p0) && in(rect.p1);
    }

    constexpr Rect frame(const QPointF & width) const noexcept
    {
        return { p0 - width, p1 + width };
    }

    constexpr Rect frame(qreal width) const noexcept
    {
        return frame(equalAxisPoint(width));
    }

    constexpr Rect zoom(const QPointF & zoom) const noexcept
    {
        const auto center = mean();
        const auto radius = mul(zoom, size()) / 2;
        return { center - radius, center + radius };
    }

    constexpr Rect move(const QPointF & move) const noexcept
    {
        return { p0 + move, p1 + move };
    }

    constexpr QRectF toQRectF() const noexcept
    {
        return { p0, p1 };
    }

    static constexpr Rect fromQRectF(const QRectF & rect_) noexcept
    {
        return { rect_.topLeft(), rect_.bottomRight() };
    }
};
