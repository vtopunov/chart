#pragma once

#include <widget/event.h>


namespace chart
{
    using widget::content_size2d;
    using widget::event_result;

    template<template<class...> class WidgetEvent, class... Args>
    using chart_event = WidgetEvent<Args..., content_size2d>;

    using mouse_wheel_event = chart_event<widget::mouse_wheel_event>;

    using gesture_event = chart_event<widget::gesture_event>;

    using mouse_double_click_event = chart_event<widget::mouse_double_click_event>;

    template<class... Args>
    using redraw_event = chart_event<widget::redraw_event, Args...>;    
}