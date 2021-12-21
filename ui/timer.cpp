#include "timer.h"

#include <core/narrow_cast.h>

using namespace std::chrono_literals;

namespace ui
{
    namespace
    {
        timer_resource create_or_set_timer(timer_resource id, timer_duration interval) noexcept
        {
            constexpr timer_duration min_duration{ USER_TIMER_MINIMUM };
            constexpr timer_duration max_duration{ USER_TIMER_MAXIMUM };

            const auto elapse = narrow_cast<UINT>(std::clamp(interval, min_duration, max_duration).count());

            const auto timer_id = SetTimer
            (
                nullptr,
                to_underlying(id),
                elapse,
                nullptr
            );

            return safe_numeric_cast<timer_resource>(timer_id);
        }
    }

    void timer_resource_deleter::operator()(timer_resource id) const noexcept
    {
        if ( has_value(id) )
        {
            D_ASSERT_WITH_SIDE_EFFECTS(KillTimer(nullptr, to_underlying(id)));
        }
    }

    timer create_timer(timer_duration interval) noexcept
    {
        return
        {
            resource_construct,
            create_or_set_timer(nulltimer, interval)
        };
    }

    timer set_timer(timer timer, timer_duration interval) noexcept
    {
        const auto new_timer_id = create_or_set_timer(timer, interval);

        if ( new_timer_id == timer )
        {
            D_UNUSED(timer.release());
        }

        return
        {
            resource_construct,
            new_timer_id
        };
    }
}
