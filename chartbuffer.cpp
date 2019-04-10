#include "chartbuffer.h"

#include <vector>
#include <qpoint.h>

ChartBuffer& ChartBuffer::default() noexcept
{
    class DefaultChartBuffer final : public ChartBuffer
    {
        span<QPointF> getPoints(size_t size) noexcept final
        {
            if (size > buffer_.capacity())
            {
                buffer_.reserve(size);
            }

            return { buffer_.data(), size };
        }

        void clear() noexcept final
        {
            buffer_.clear();
        }

        std::vector<QPointF> buffer_;
    };

    static DefaultChartBuffer buffer;
    return buffer;
}
