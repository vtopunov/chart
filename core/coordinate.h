#pragma once

#include "rect.h"
#include "lerp.h"

enum class coordiante_system
{
    windows,
    math
};

template<coordiante_system system>
constexpr auto other_system_v = ( system == coordiante_system::windows ) ? coordiante_system::math : coordiante_system::windows;

namespace coordiante_validation
{
    struct validation
    {
        struct rect_validation : public rect_t
        {
            constexpr rect_validation(real_t min, real_t max) noexcept
                : rect{ point_t::fill(min), point_t::fill(max) }
            {}
        };

        rect_validation point_validate;
        rect_validation size_validate;

        constexpr bool operator () (rect_t rect) const noexcept
        {
            return point_validate.includes(rect) && size_validate.includes(rect.size());
        }
    };

    template<coordiante_system>
    struct validation_for
    {
        static constexpr validation validation;
    };

    template<>
    struct validation_for<coordiante_system::windows>
    {
        static constexpr auto MAX = std::numeric_limits<int>::max();

        static constexpr validation validation
        {
            { 0, MAX },
            { 3, MAX }
        };
    };

    template<>
    struct validation_for<coordiante_system::math>
    {
        static constexpr real_t EPS_RANK{ 100 };
        static constexpr real_t MAX{ FLT_MAX / EPS_RANK };
        static constexpr real_t EPS{ EPS_RANK * FLT_EPSILON };

        static constexpr validation validation
        {
            { -MAX, MAX },
            {  EPS, MAX }
        };
    };

    template<coordiante_system system>
    constexpr validation validate_for = validation_for<system>::validation;
}

template<coordiante_system system>
struct coordiante_inverse_axis
{
    constexpr rect_t operator () (rect_t rect) const noexcept
    {
        return rect;
    }
};

template<>
struct coordiante_inverse_axis<coordiante_system::math>
{
    constexpr rect_t operator () (rect_t rect) const noexcept
    {
        return rect.with_inverse_axis<axis_type::Y>();
    }
};

struct coordinate_transformation
{
    polynomial<real_t> x;
    polynomial<real_t> y;

    constexpr coordinate_transformation(rect_t from, rect_t to) noexcept
        : x{ lerp(from.x_axis_range(), to.x_axis_range()) }
        , y{ lerp(from.y_axis_range(), to.y_axis_range()) }
    {}

    constexpr point_t operator () (point_t point) const noexcept
    {
        return { x(point.x()), y(point.y()) };
    }
};

template<coordiante_system system_>
class coordiante_rect
{
private:
    struct UnsafeConstructor
    {};

    constexpr coordiante_rect(rect_t rect, UnsafeConstructor) noexcept
        : rect_{ rect }
    {}

public:
    static constexpr auto system = system_;
    static constexpr auto other_system = other_system_v<system>;
    using other_coordinate_rect = coordiante_rect<other_system>;

    constexpr coordiante_rect() noexcept = default;

    constexpr bool set(rect_t rect) noexcept
    {
        coordiante_rect temp{ rect, UnsafeConstructor{} };

        if ( temp )
        {
            *this = temp;
            return true;
        }

        return false;
    }

    explicit constexpr operator bool() const noexcept
    {
        using namespace coordiante_validation;
        return validate_for<system>(rect_);
    }

    constexpr rect_t rect_for_transformation() const noexcept
    {
        constexpr auto inverse_axis = coordiante_inverse_axis<system>();
        return inverse_axis(rect_);
    }

    constexpr coordinate_transformation to(const other_coordinate_rect& rect) const noexcept
    {
        return { rect_for_transformation(), rect.rect_for_transformation() };
    }

    constexpr rect_t rect() const noexcept
    {
        return rect_;
    }

    constexpr bool zoom(point_t zoom) noexcept
    {
        return set(rect_.with_zooming(zoom));
    }

    constexpr bool move(point_t move) noexcept
    {
        return set(rect_.with_moving(move));
    }

private:
    rect_t rect_{};
};