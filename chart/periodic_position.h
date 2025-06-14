#pragma once

#include <core/point2d.h>


namespace chart
{
    struct periodic_position
    {
        point2re begin;
        point2re repeat;
    };

    struct periodic_value_position
    {
        periodic_position value;
        periodic_position px;
        point2d<size_t> count;
    };
}
