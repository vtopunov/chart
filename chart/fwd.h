#pragma once


#include <widget/fwd.h>


namespace chart
{
    using px::real_t;
    using px::real_vec2;
    using px::real_size2d;
    using px::real_point2d;
    using px::real_point2d_cspan;

    constexpr auto real_inf = numeric_inf_v<real_t>;
    constexpr auto real_lowest_inf = -real_inf;
    constexpr auto real_point2d_inf = fill_to<point2d>(real_inf);
    constexpr auto real_point2d_lowest_inf = fill_to<point2d>(real_lowest_inf);

    using widget::stretchable_pxrectangle;
    using widget::event_result;
    using namespace widget::event_declaration;

    template<class Tuple>
    struct subitems;
}