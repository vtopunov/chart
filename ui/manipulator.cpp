#include "manipulator.h"
   
#include <ui/event.h>


namespace ui
{
    namespace manipulator
    {
        namespace
        {
            [[nodiscard]]
            constexpr real_t inner_product(point2re v0, point2re v1) noexcept
            {
                return v0.x() * v1.x() + v0.y() * v1.y();
            };

            [[nodiscard]]
            constexpr bool cache_has_value(real_t value) noexcept
            {
                return no_cached_value != value;
            }

            template<class T>
            [[nodiscard]] constexpr bool cache_has_value(const vec2<T>& v) noexcept
            {
                return cache_has_value(v._0)
                    && cache_has_value(v._1);
            }

            [[nodiscard]]
            constexpr size_t size(const vpoint_cache& v) noexcept
            {
                size_t result{ 0u };

                if (cache_has_value(v._0._0))
                {
                    ++result;
                    if (cache_has_value(v._1._0))
                    {
                        ++result;
                    }
                }

                return result;
            }

            [[nodiscard]]
            constexpr bool cache_is_valid(const vpoint_cache& v) noexcept
            {
                switch (size(v))
                {
                    case 0u: return (no_cached_vpoint == v);
                    case 1u: return (cache_has_value(v._0) && (no_cached_point == v._1));
                    case 2u: return cache_has_value(v);

                    default:
                        break;
                }

                return false;
            }

            enum class orientation
            {
                diagonally = 0,
                horizontally = (1 << 0),
                vertically = (1 << 1)
            };

            [[nodiscard]]
            constexpr orientation locating_orientation(point2re abs_dp) noexcept
            {
                constexpr real_t threshold{ 100.0 };

                orientation result{ orientation::diagonally };

                if (abs_dp.x() < threshold)
                {
                    e_bit_or_eq(result, orientation::vertically);
                }

                if (abs_dp.y() < threshold)
                {
                    e_bit_or_eq(result, orientation::horizontally);
                }

                return result;
            }

            [[nodiscard]]
            constexpr orientation prepare(vpoint_cache& v) noexcept
            {
                const auto dp = v._1 - v._0;
                const auto abs_dp = md_abs(dp);

                {
                    const auto along_x = abs_dp.y() < abs_dp.x();
                    const auto is_codirectional = is_positive_or_epsfp((along_x) ? dp.x() : dp.y());
                    v = (is_codirectional) ? v : inverse(v);
                }

                return locating_orientation(abs_dp);
            }

            [[nodiscard]]
            constexpr orientation prepare(vpoint_cache& v, const vpoint_cache& order) noexcept
            {
                const auto dp = (v._1 - v._0);

                {
                    const auto is_codirectional = is_positive_or_epsfp(inner_product(dp, order._1 - order._0));
                    v = (is_codirectional) ? v : inverse(v);
                }

                return locating_orientation(md_abs(dp));
            }


            [[nodiscard]]
            constexpr gesture make_gesture(vpoint_cache v0, vpoint_cache v1) noexcept
            {
                if (const auto ov = prepare(v0); ov == prepare(v1, v0))
                {
                    switch (ov)
                    {
                        case orientation::diagonally:
                            return
                            {
                                .transformation{ make_transformation(v0, v1) }
                            };

                        case orientation::horizontally:
                            return
                            {
                                .transformation
                                {
                                    .fx{ make_transformation_fx(v0, v1) },
                                    .fy{ gesture::no_transformation_function }
                                }
                            };

                        case orientation::vertically:
                            return
                            {
                                .transformation
                                {
                                    .fx{ gesture::no_transformation_function },
                                    .fy{ make_transformation_fy(v0, v1) }
                                }
                            };

                        default:
                            break;
                    }
                }

                return no_gesture;
            }

            [[nodiscard]]
            constexpr gesture make_gesture(point2re p0, point2re p1) noexcept
            {
                return
                {
                    .transformation
                    {
                        .fx
                        {
                            .a1{ gesture::no_transformation_scale },
                            .a0{ p1.x() - p0.x() }
                        },
                        .fy
                        {
                            .a1{ gesture::no_transformation_scale },
                            .a0{ p1.y() - p0.y() }
                        }
                    }
                };
            }

            [[nodiscard]]
            constexpr gesture make_gesture_opt(const vpoint_cache& v0, const vpoint_cache& v1) noexcept
            {
                return (2u == size(v0)) ? make_gesture(v0, v1) : no_gesture;
            }

            [[nodiscard]]
            constexpr gesture make_gesture_opt(const vpoint_cache& v, point2re p) noexcept
            {
                return (1u == size(v)) ? make_gesture(v._0, p) : no_gesture;
            }

            [[nodiscard]]
            constexpr gesture new_manipulation(vpoint_cache& cached_p, vpoint_cache new_p) noexcept
            {
                return make_gesture_opt(std::exchange(cached_p, new_p), new_p);
            }

            [[nodiscard]]
            constexpr gesture new_manipulation(vpoint_cache& cached_p, point2re new_p) noexcept
            {
                return make_gesture_opt
                (
                    std::exchange(cached_p, vec2{ new_p, no_cached_point }),
                    new_p
                );
            }

            [[nodiscard]]
            constexpr point2re to_cached_point(const ui::pointer_event::point2d_type& p) noexcept
            {
                return
                {
                    numeric_cast<real_t>(p.x()),
                    numeric_cast<real_t>(p.y())
                };
            }
        }

        gesture new_manipulation(vpoint_cache& cached_p, const ui::pointer_event& e) noexcept
        {
            D_ASSERT(cache_is_valid(cached_p));

            switch (e.size())
            {
                case 1_uz: return new_manipulation(cached_p, to_cached_point(e.pointer()));
                case 2_uz:
                    return new_manipulation
                    (
                        cached_p,
                        vpoint_cache
                        {
                            to_cached_point(e.pointer(0u)),
                            to_cached_point(e.pointer(1u))
                        }
                    );

                default:
                    break;
            }

            cached_p = no_cached_vpoint;
            return no_gesture;
        }
    }
}
