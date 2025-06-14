#pragma once

#include <core/transformation.h>

#include <ui/fwd.h>


namespace ui
{
    namespace manipulator
    {
        struct gesture
        {
            static constexpr real_t no_transformation_scale{ 1.0 };
            static constexpr real_t no_transformation_shift{ 0.0 };

            using transformation_type = transformation<real_t>;

            static constexpr transformation_type::function_type no_transformation_function
            {
                .a1{ no_transformation_scale },
                .a0{ no_transformation_shift }
            };

            static constexpr transformation_type no_transformation
            {
                no_transformation_function,
                no_transformation_function
            };

            transformation_type transformation;

            [[nodiscard]]
            constexpr point2re shift() const noexcept
            {
                return transformation.shift();
            }

            [[nodiscard]]
            constexpr size2re scale() const noexcept
            {
                return transformation.scale();
            }

            template<class T>
            [[nodiscard]] constexpr T transformation_as(const T& v) const noexcept
            {
                return md_round_to_near(transformation(v), v);
            }
        };

        constexpr gesture no_gesture{ gesture::no_transformation };

        using vpoint_cache = vec2<point2re>;

        constexpr auto no_cached_value = numeric_inf_v<real_t>;
        constexpr auto no_cached_point = fill_to<point2re>(no_cached_value);
        constexpr auto no_cached_vpoint = fill_to<vpoint_cache>(no_cached_point);

        [[nodiscard]]
        gesture new_manipulation(vpoint_cache& cached_p, const ui::pointer_event& e) noexcept;
    }

    using manipulator::no_gesture;

    using user_vpoint_cache = manipulator::vpoint_cache;
    constexpr auto no_cached_user_vpoint = manipulator::no_cached_vpoint;

    using manipulator::new_manipulation;
}


