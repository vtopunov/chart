#include "timer.h"

#include <core/narrow.h>

#include <os/os.h>

using namespace std::chrono_literals;

namespace ui
{
    namespace
    {
        using native_timer_id_t = UINT_PTR;

        constexpr native_timer_id_t to_native(timer_resource id) noexcept
        {
            static_assert(std::is_same_v<std::underlying_type_t<timer_resource>, native_timer_id_t>);
            return static_cast<native_timer_id_t>(id);
        }

        constexpr timer_resource from_native(native_timer_id_t id) noexcept
        {
            static_assert(std::is_same_v<std::underlying_type_t<timer_resource>, native_timer_id_t>);
            return static_cast<timer_resource>(id);
        }

        timer_resource create_or_set_timer(timer_resource id, timer_duration interval) noexcept
        {
            constexpr timer_duration min_duration{ USER_TIMER_MINIMUM };
            constexpr timer_duration max_duration{ USER_TIMER_MAXIMUM };

            const auto elapse = narrow_cast<UINT>(std::clamp(interval, min_duration, max_duration).count());

            const auto timer_id = SetTimer
            (
                nullptr,
                to_native(id),
                elapse,
                nullptr
            );

            return from_native(timer_id);
        }
    }

    void timer_resource_deleter::operator()(timer_resource id) const noexcept
    {
        if ( has_value(id) )
        {
            D_ASSERT_WITH_SIDE_EFFECTS(KillTimer(nullptr, to_native(id)));
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

        if (  new_timer_id == timer.r() )
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
