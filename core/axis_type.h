#pragma once

#include <type_traits>

enum class axis_type : size_t
{
    X,
    Y
};

template <axis_type axis>
using axis_constant = std::integral_constant<axis_type, axis>;

using x_axis_type = axis_constant<axis_type::X>;
using y_axis_type = axis_constant<axis_type::Y>;

template<axis_type axis>
constexpr auto other_axis_v = ( axis == axis_type::X ) ? axis_type::Y : axis_type::X;
