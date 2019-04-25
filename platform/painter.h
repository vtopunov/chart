#pragma once

#include <core/rect.h>
#include <core/color.h>
#include <core/span.h>


class painter
{
public:
    constexpr painter(void* context) noexcept
        : context(context)
    {}

    void pen(color color) noexcept;
    void brush(color color) noexcept;
    void draw_rect(rect_t rect) noexcept;
    void clip(rect_t rect) noexcept;
    void antialiasing(bool enable) noexcept;
    void draw_polyline(span<const point_t> polyline) noexcept;

private:
    void* context;
};
