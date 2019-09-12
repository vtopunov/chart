#include <array>

#include <core/span.h>
#include <core/lerp.h>
#include <core/numerical_range.h>
#include <core/point.h>
#include <core/math_constants.h>
#include <core/buffer.h>

#pragma warning(push, 0)
#pragma warning(disable: 26451)
#include <3rdparty/gcem/include/gcem.hpp>
#pragma warning(pop)

#include <core/assert.h>

constexpr double sinc( double x ) noexcept
{
    return ( gcem::abs( x ) > std::numeric_limits<double>::epsilon() ) ? ( gcem::sin( x ) / x ) : ( 1.0 );
};

constexpr span<point_t> fn_generate( span<point_t> points, double ( *fn ) ( double ) ) noexcept
{
    assert( fn );

    const size_t size = points.size();
    const auto abscissa_max = 5 * pi_v<real_t>;
    const numerical_range<real_t> abscissa_range{ -abscissa_max, abscissa_max };
    const numerical_range<size_t> index_range{ 0_z, size - 1_z };
    const auto abscissa = lerp( index_range, abscissa_range );

    for ( size_t i = index_range.front(); i <= index_range.back(); ++i )
    {
        const auto x = abscissa( i );
        points[i] = point_t{ x, fn( x ) };
    }

    return points;
};

constexpr auto sinc_tbl = [] ()
{
    std::array<point_t, 100_z> temp{};
    fn_generate( temp, sinc );
    return temp;
}();

void test_buffer() noexcept
{
    const auto points = buffer::default_instance().get<point_t>( sinc_tbl.size() );

    assert( points.size() == sinc_tbl.size() );
    for ( size_t i = 0; i < sinc_tbl.size(); ++i )
    {
        points[i] = sinc_tbl[i];
    }

    assert( !memcmp( points.data(), sinc_tbl.data(), sinc_tbl.size() ) );
}
