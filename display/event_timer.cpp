#include "event_timer.h"

using namespace std::chrono_literals;

namespace display
{
    void event_timer_resource_deleter::operator()(event_timer_resource id, resource_destroy_t) const noexcept
    {
        if ( has_value(id) )
        {
            D_ASSERT_WITH_SIDE_EFFECTS(KillTimer(nullptr, to_underlying(id)));
        }
    }

    namespace
    {
        event_timer_resource set_timer(event_timer_resource id, event_timer_duration interval) noexcept
        {
            constexpr event_timer_duration min_duration{ USER_TIMER_MINIMUM };
            constexpr event_timer_duration max_duration{ USER_TIMER_MAXIMUM };

            const auto elapse = narrow_cast<UINT>( std::clamp(interval, min_duration, max_duration).count() );

            const auto timer_id = SetTimer
            (
                nullptr,
                to_underlying(id),
                elapse,
                nullptr
            );

            return underlying_cast<event_timer_resource>( timer_id );
        }
    }

    event_timer create_event_timer(event_timer_duration interval) noexcept
    {
        return
        {
            resource_construct,
            set_timer(nulleventtimer, interval)
        };
    }

    event_timer set_event_timer(event_timer timer, event_timer_duration interval) noexcept
    {
        const auto new_timer_id = set_timer(timer, interval);

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
