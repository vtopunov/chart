#pragma once

#include "span.h"

class QPointF;

class ChartBuffer
{
public:
    virtual ~ChartBuffer() noexcept = default;

    virtual span<QPointF> getPoints(size_t size) noexcept = 0;

    virtual void clear() noexcept = 0;

    static ChartBuffer& default() noexcept;
};