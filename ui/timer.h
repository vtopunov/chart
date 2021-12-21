#pragma once

#include <chrono>

#include <core/resouce.h>

#include <ui/timer_fwd.h>

namespace ui
{
    struct timer_resource_deleter
    {
        void operator () (timer_resource timer) const noexcept;
    };

    using timer = unique_resource<timer_resource, timer_resource_deleter>;

    using timer_duration = std::chrono::milliseconds;

    [[nodiscard]]
    timer create_timer(timer_duration interval) noexcept;

    [[nodiscard]]
    timer set_timer(timer timer, timer_duration interval) noexcept;
}