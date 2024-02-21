#pragma once

#include <core/transformation.h>
#include <core/small_vector.h>

#include <px/fwd.h>


namespace chart
{
    using px::real_t;
    using px::real_vec2;
    using px::real_point2d;
    using px::real_point2d_cspan;
    using real_vpoint2d = small_vector<real_point2d>;

    using space_diagonal_t = vec2<real_point2d>;

    using transformation_t = decltype(::make_transformation(std::declval<const space_diagonal_t&>(), std::declval<const space_diagonal_t&>()));
}