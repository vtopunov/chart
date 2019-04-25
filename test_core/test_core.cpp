#include <array>
#include <functional>
#include <iostream>

#include <core/util.h>
#include <core/math_constants.h>
#include <core/vec.h>
#include <core/numerical_range.h>
#include <core/point.h>
#include <core/rect.h>
#include <core/polynom.h>
#include <core/lerp.h>
#include <core/rational.h>
#include <core/color.h>
#include <core/buffer.h>

#include <3rdparty/gcem/include/gcem.hpp>

constexpr double sinc( double x ) noexcept
{
    return ( gcem::abs( x ) > std::numeric_limits<double>::epsilon() ) ? ( gcem::sin( x ) / x ) : ( 1.0 );
}

constexpr span<point_t> fn_generate( span<point_t> result, double ( *fn ) ( double ) )
{
    assert( fn );

    const size_t size = result.size();
    const auto abscissa_max = 5 * pi_v<real_t>;
    const numerical_range<real_t> abscissa_range{ -abscissa_max, abscissa_max };
    const numerical_range<size_t> index_range{ 0_z, size - 1_z };
    const auto abscissa = lerp( index_range, abscissa_range );

    for ( size_t i = index_range.front(); i <= index_range.back(); ++i )
    {
        const auto x = abscissa( i );
        result[i] = point_t{ x, fn( x ) };
    }

    return result;
}

constexpr auto sinc_tbl = [] ()
{
    std::array<point_t, 100_z> result{};
    fn_generate( result, sinc );
    return result;
}( );

int main() noexcept
{
    {
        static_assert( !is_safe_narrowing_conversion<uint32_t>( -1L ) );
        static_assert( !is_safe_narrowing_conversion<int16_t>( -0x8001L ) );
        static_assert( !is_safe_narrowing_conversion<int16_t>( 0x8000L ) );
        static_assert( !is_safe_narrowing_conversion<uint16_t>( -1L ) );
        static_assert( !is_safe_narrowing_conversion<uint16_t>( 0x10000L ) );
        static_assert( !is_safe_narrowing_conversion<int32_t>( 0xffffffffUL ) );
    }

    {
        constexpr vec v0{ 1, 1 };
        constexpr vec v1{ 1, 2 };
        constexpr vec v2{ 2, 1 };
        constexpr vec v3{ 2, 2 };
        constexpr vec v4{ 1, 0 };
        constexpr vec v5{ 3, 1 };

        static_assert( v0 != v1 );
        static_assert( !( v0 < v1 ) );
        static_assert( !( v0 <= v1 ) );
        static_assert( !( v1 < v0 ) );
        static_assert( !( v1 <= v0 ) );
        static_assert( !( v0 > v1 ) );
        static_assert( !( v0 >= v1 ) );
        static_assert( !( v1 > v0 ) );
        static_assert( !( v1 >= v0 ) );

        static_assert( max( v1, v2 ) == v3 );
        static_assert( v3 > v0 );
        static_assert( v3 >= v0 );
        static_assert( v0 < v3 );
        static_assert( v0 <= v3 );

        static_assert( min( v4, v2 ) == v4 );
        static_assert( v4 < v2 );
        static_assert( v4 <= v2 );
        static_assert( v2 > v4 );
        static_assert( v2 >= v4 );

        static_assert( v1.with_reverse() == v2 );
        static_assert( v2.with_reverse() == v1 );
        static_assert( 2 * v0 == v0 * 2 );
        static_assert( 2 * v0 == v3 );
        static_assert( v3 / 2 == v0 );
        static_assert( v2 + v4 == v5 );
        static_assert( v4 + v2 == v5 );
        static_assert( v5 - v2 == v4 );
        static_assert( v5 - v4 == v2 );
        static_assert( -( -v5 ) == v5 );
        static_assert( -( -( -v5 ) ) == -v5 );
        static_assert( -( v4 + ( -v5 ) ) == v2 );
    }

    {
        constexpr numerical_range range{ 1, 3 };
        static_assert( range.min() == 1 );
        static_assert( range.max() == 3 );
        static_assert( range.front() == 1 );
        static_assert( range.back() == 3 );
        static_assert( !range.includes( 0 ) );
        static_assert( range.includes( 1 ) );
        static_assert( range.includes( 2 ) );
        static_assert( range.includes( 3 ) );
        static_assert( !range.includes( 4 ) );

        {
            constexpr auto inverse_range = range.with_inverse();
            static_assert( inverse_range.min() == 1 );
            static_assert( inverse_range.max() == 3 );
            static_assert( inverse_range.front() == 3 );
            static_assert( inverse_range.back() == 1 );
        }

        {
            constexpr auto range_with_4 = range.with_inclusion( 4 );
            static_assert( range_with_4.includes( 4 ) );
            static_assert( range_with_4.includes( 1 ) );
            static_assert( !range_with_4.includes( 0 ) );
        }

        {
            constexpr auto range_moving_1 = range.with_moving( 1 );
            static_assert( range_moving_1.includes( 4 ) );
            static_assert( !range_moving_1.includes( 1 ) );
        }
    }

    {
        constexpr point p0{ 3, 5 };
        constexpr point p0_minus{ -3, -5 };
        static_assert( p0.x() == 3 );
        static_assert( p0.y() == 5 );
        static_assert( p0.get<axis_type::X>() == p0.x() );
        static_assert( p0.get<axis_type::Y>() == p0.y() );

        constexpr point p1{ 1, 2 };
        constexpr point p2{ 2, 3 };
        static_assert( p1 + p2 == p0 );
        static_assert( p2 + p1 == p0 );
        static_assert( -p0 == p0_minus );
        static_assert( -( -p0 ) == p0 );
        static_assert( p0 - p1 == p2 );
        static_assert( p0 - p2 == p1 );
        static_assert( p1 - p0 == -p2 );
        static_assert( p1 * 2 == p1 + p1 );
        static_assert( 2 * p1 == p1 + p1 );
        static_assert( ( p0 + p1 + p2 ) / 2 == p0 );
    }

    {
        constexpr numerical_range r0{ 1, 3 };
        constexpr numerical_range r1{ 2, 5 };
        constexpr point min{ 1, 2 };
        constexpr point max{ 3, 5 };
        constexpr point size{ max - min };
        constexpr rect rc{ min, max };
        constexpr rect rc_range{ r0, r1 };
        static_assert( rc == rc_range );

        static_assert( rc.diagonal.front() == min );
        static_assert( rc.diagonal.back() == max );
        static_assert( rc.includes( min ) );
        static_assert( rc.includes( max ) );
        static_assert( !rc.includes( max / 3 ) );
        static_assert( rc.with_inclusion( max / 3 ).includes( max / 3 ) );
        static_assert( rc.axis_range<axis_type::X>() == r0 );
        static_assert( rc.axis_range<axis_type::Y>() == r1 );
        static_assert( rc.x_axis_range() == r0 );
        static_assert( rc.y_axis_range() == r1 );
        static_assert( rc.size() == size );
        static_assert( rc.width() == size.x() );
        static_assert( rc.height() == size.y() );
        static_assert( rc.with_inverse_axis<axis_type::X>().x_axis_range() == r0.with_inverse() );
        static_assert( rc.with_inverse_axis<axis_type::Y>().y_axis_range() == r1.with_inverse() );
    }

    {
        constexpr polynom line_function{ 1, 2 };
        constexpr point p0{ 0, 1 };
        constexpr point p1{ 1, 3 };
        constexpr point p2{ 2, 5 };

        constexpr auto line_function_01 = lerp( p0, p1 );
        constexpr auto line_function_12 = lerp( p1, p2 );
        constexpr auto line_function_02 = lerp( p0, p2 );
        static_assert( line_function.coefficients == line_function_01.coefficients );
        static_assert( line_function.coefficients == line_function_12.coefficients );
        static_assert( line_function.coefficients == line_function_02.coefficients );
        static_assert( line_function_01( p2.x() ) == p2.y() );
        static_assert( line_function_12( p0.x() ) == p0.y() );
        static_assert( line_function_02( p1.x() ) == p1.y() );
    }

    {
        constexpr rational_t r0{ 2, 3 };
        constexpr rational_t r1{ 3, 4 };
        constexpr rational_t r2{ 8, 10 };
        constexpr rational_t r3 = simplify( r2 );
        static_assert( r3 == rational_t( 4, 5 ) );
        static_assert( -r3 == rational_t( -4, 5 ) );
        static_assert( r0 + r1 == rational_t( 17, 12 ) );
        static_assert( r1 + r0 == r0 + r1 );
        static_assert( r0 - r1 == rational_t( -1, 12 ) );
        static_assert( r1 - r0 == rational_t( 1, 12 ) );
        static_assert( -r0 + r1 == r1 - r0 );
        static_assert( r0 * r1 == rational_t( 1, 2 ) );
        static_assert( r1 * r0 == r0 * r1 );
        static_assert( r0 / r1 == rational_t( 8, 9 ) );
        static_assert( ( r0 * r1 ) / r1 == r0 );
    }

    {
        constexpr numerical_range animation_index{ 0ll, 8ll };
        constexpr numerical_range colors_from_cyan_to_red{ colors::cyan, colors::red };
        constexpr auto animation_color_cyan_to_red = lerp( animation_index, colors_from_cyan_to_red );

        constexpr auto c0 = animation_color_cyan_to_red( 0 );
        constexpr auto c1 = animation_color_cyan_to_red( 1 );
        constexpr auto c2 = animation_color_cyan_to_red( 2 );
        constexpr auto c4 = animation_color_cyan_to_red( 4 );
        constexpr auto c6 = animation_color_cyan_to_red( 6 );
        constexpr auto c8 = animation_color_cyan_to_red( 8 );

        static_assert( c0 == colors::cyan );
        static_assert( c1 == color( 31, 223, 223 ) );
        static_assert( c2 == color( 63, 191, 191 ) );
        static_assert( c4 == color( 127, 127, 127 ) );
        static_assert( c6 == color( 191, 63, 63 ) );
        static_assert( c8 == colors::red );
    }

    {
        const auto points = buffer::default_instance().get<point_t>( sinc_tbl.size() );

        assert( points.size() == sinc_tbl.size() );
        for ( size_t i = 0; i < sinc_tbl.size(); ++i )
        {
            points[i] = sinc_tbl[i];
        }

        assert( !memcmp( points.data(), sinc_tbl.data(), sinc_tbl.size() ) );
    }
}
