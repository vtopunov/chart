#pragma once

#include <px/fwd.h>


namespace chart
{
    struct periodic_position
    {
        px::real_point2d begin;
        px::real_point2d repeat;
    };

    struct periodic_value_position
    {
        periodic_position value;
        periodic_position px;
        point2d<size_t> count;
    };
}
