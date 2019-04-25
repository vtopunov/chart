#pragma once

#include <qcolor.h>
#include <qpoint.h>
#include <qrect.h>

#include <core/span.h>
#include <core/rect.h>
#include <core/color.h>

template<class Target>
struct platform_caster
{
    template<class Source>
    static Target cast(Source source) noexcept
    {
        return static_cast<Target>( source );
    }
};

template<>
struct platform_caster<QColor>
{
    static QColor cast(color source) noexcept
    {
        return QColor(source.red, source.green, source.blue, source.alpha);
    }
};

template<>
struct platform_caster<QPointF>
{
    static QPointF cast(point_t source) noexcept
    {
        return { source.x(), source.y() };
    }
};

template<>
struct platform_caster<QRectF>
{
    static QRectF cast(rect_t source) noexcept
    {
        using point_caster = platform_caster<QPointF>;
        return 
        { 
            point_caster::cast( source.diagonal.front() ),
            point_caster::cast( source.diagonal.back() )
        };
    }
};

template<>
struct platform_caster<point_t>
{
    static point_t cast(QPointF source) noexcept
    {
        return { source.x(), source.y() };
    }
};

template<>
struct platform_caster<rect_t>
{
    static rect_t cast(QRectF source) noexcept
    {
        using point_caster = platform_caster<point_t>;
        return
        {
            point_caster::cast(source.topLeft()), 
            point_caster::cast(source.bottomRight())
        };
    }
};


template<>
struct platform_caster<span<const QPointF>>
{
    static span<const QPointF> cast(span<const point_t> source) noexcept
    {
#pragma warning(push)
#pragma warning(disable : 26490) // Don't use reinterpret_cast
        static_assert( sizeof(point_t) == sizeof(QPointF) );
        return { reinterpret_cast<const QPointF*>( source.data() ), source.size() };
#pragma warning(pop)
    }
};

template<class Target, class Source>
Target platform_cast(Source source) noexcept
{
    return platform_caster<Target>::cast(source);
}
