#include "chartfiguresrect.h"
#include "chartwidgetrect.h"

namespace
{
    struct RectValidation
    {
        struct Validation
        {
            Rect rect;

            constexpr Validation(qreal min, qreal max) noexcept
                : rect{ equalAxisPoint(min), equalAxisPoint(max) }
            {}
        };

        Validation pointValidate;
        Validation sizeValidate;

        constexpr bool operator () (const Rect& rect_) const noexcept
        {
            return pointValidate.rect.in(rect_) && sizeValidate.rect.in(rect_.p1 - rect_.p0);
        }
    };

    constexpr auto PIXEL_MAX = std::numeric_limits<int>::max();

    constexpr RectValidation widgetRectValidate
    {
        { 0, PIXEL_MAX },
        { 3, PIXEL_MAX }
    };

    constexpr qreal VALUE_EPS_RANK{ 100 };
    constexpr qreal VALUE_MIN{ -FLT_MAX / VALUE_EPS_RANK };
    constexpr qreal VALUE_MAX{ FLT_MAX / VALUE_EPS_RANK };
    constexpr qreal VALUE_EPS{ VALUE_EPS_RANK * FLT_EPSILON };

    constexpr RectValidation figuresRectValidate
    {
        { VALUE_MIN, VALUE_MAX },
        { VALUE_EPS, VALUE_MAX }
    };
}

bool ChartFiguresRect::set(const Rect& rect) noexcept
{
    const auto isValidNewRect = figuresRectValidate(rect);
    if (isValidNewRect)
    {
        rect_ = rect;
    }
    return isValidNewRect;
}

bool ChartWidgetRect::set(const Rect& rect) noexcept
{
    const auto isValidNewRect = widgetRectValidate(rect);
    if (isValidNewRect)
    {
        rect_ = rect;
    }
    return isValidNewRect;
}

bool ChartFiguresRect::isValid() const noexcept
{
    return figuresRectValidate(rect_);
}

bool ChartWidgetRect::isValid() const noexcept
{
    return widgetRectValidate(rect_);
}
