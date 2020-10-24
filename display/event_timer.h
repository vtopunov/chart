#pragma once

#include <chrono>

#include <core/resouce.h>

#include <display/defs.h>

namespace display
{
    using event_timer_duration = std::chrono::milliseconds;

    enum class event_timer_resource : UINT_PTR
    {};

    using nulleventtimer_t = null_t<event_timer_resource>;

    inline constexpr nulleventtimer_t nulleventtimer{};

    struct event_timer_resource_deleter
    {
        void operator () (event_timer_resource timer, resource_destroy_t) const noexcept;
    };

    using event_timer = unique_resource<event_timer_resource, event_timer_resource_deleter>;

    [[nodiscard]]
    event_timer create_event_timer(event_timer_duration interval) noexcept;

    [[nodiscard]]
    event_timer set_event_timer(event_timer timer, event_timer_duration interval) noexcept;
}