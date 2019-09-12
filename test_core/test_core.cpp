#include <array>
#include <set>
#include <functional>
#include <iostream>
#include <compare>

#include <core/util.h>
#include <core/narrow_cast.h>
#include <core/underlying_cast.h>
#include <core/is_detected.h>
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
#include <core/safe_handle.h>
#include <core/small_flat_map.h>

#pragma warning(push, 0)
#pragma warning(disable: 26451)
#include <3rdparty/gcem/include/gcem.hpp>
#pragma warning(pop)

#include <core/assert.h>

namespace test_is_detected
{
    struct struct_with_method
    {
        void method() const noexcept {}
    };

    template<class T>
    using has_method = decltype( std::declval<T>().method(), 0 );
}

extern void test_narrow_cast() noexcept;
extern void test_underlying_cast() noexcept;
extern void test_vec() noexcept;
extern void test_numerical_range() noexcept;
extern void test_point() noexcept;
extern void test_rect() noexcept;
extern void test_lerp() noexcept;
extern void test_rational() noexcept;
extern void test_lerp_color() noexcept;
extern void test_handle() noexcept;
extern void test_small_flat_map() noexcept;

int main() noexcept
{
    test_narrow_cast();
    test_underlying_cast();
    test_vec();
    test_numerical_range();
    test_point();
    test_rect();
    test_lerp();
    test_rational();
    test_handle();
    test_small_flat_map();


    

    {
       

    {
        constexpr auto sinc = [] ( double x )
        {
            return ( gcem::abs( x ) > std::numeric_limits<double>::epsilon() ) ? ( gcem::sin( x ) / x ) : ( 1.0 );
        };

        constexpr auto fn_generate = [] ( span<point_t> points, double ( *fn ) ( double ) )
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
        }( );

        const auto points = buffer::default_instance().get<point_t>( sinc_tbl.size() );

        assert( points.size() == sinc_tbl.size() );
        for ( size_t i = 0; i < sinc_tbl.size(); ++i )
        {
            points[i] = sinc_tbl[i];
        }

        assert( !memcmp( points.data(), sinc_tbl.data(), sinc_tbl.size() ) );
    }
}
