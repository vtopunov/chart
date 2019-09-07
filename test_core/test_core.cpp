#include <array>
#include <set>
#include <functional>
#include <iostream>
#include <compare>

#include <core/util.h>
#include <core/narrow_cast.h>
#include <core/underlying_cast.h>
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
#include <core/static_map.h>

#pragma warning(push, 0)
#pragma warning(disable: 26451)
#include <3rdparty/gcem/include/gcem.hpp>
#pragma warning(pop)

#include <core/assert.h>

constexpr double sinc( double x ) noexcept
{
    return ( gcem::abs( x ) > std::numeric_limits<double>::epsilon() ) ? ( gcem::sin( x ) / x ) : ( 1.0 );
}

constexpr span<point_t> fn_generate( span<point_t> points, double ( *fn ) ( double ) )
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
}

constexpr auto sinc_tbl = [] ()
{
    std::array<point_t, 100_z> temp{};
    fn_generate( temp, sinc );
    return temp;
}( );



int main() noexcept
{
    {
        static_assert( !is_safe_integral_conversion_v<uint32_t, int32_t> );
        static_assert( !is_safe_integral_conversion_v<int32_t, uint32_t> );
        static_assert( is_safe_integral_conversion_v<int32_t, int32_t> );
        static_assert( is_safe_integral_conversion_v<uint32_t, uint32_t> );

        static_assert( !is_safe_integral_conversion_v<uint16_t, int32_t> );
        static_assert( !is_safe_integral_conversion_v<uint16_t, uint32_t> );
        static_assert( !is_safe_integral_conversion_v<int16_t, int32_t> );
        static_assert( !is_safe_integral_conversion_v<int16_t, uint32_t> );

        static_assert( !is_safe_integral_conversion_v<uint32_t, int16_t> );
        static_assert( is_safe_integral_conversion_v<uint32_t, uint16_t> );
        static_assert( is_safe_integral_conversion_v<int32_t, int16_t> );
        static_assert( is_safe_integral_conversion_v<int32_t, uint16_t> );

        static_assert( !is_safe_narrowing_conversion<uint32_t>( -1L ) );
        static_assert( !is_safe_narrowing_conversion<int16_t>( -0x8001L ) );
        static_assert( !is_safe_narrowing_conversion<int16_t>( 0x8000L ) );
        static_assert( !is_safe_narrowing_conversion<uint16_t>( -1L ) );
        static_assert( !is_safe_narrowing_conversion<uint16_t>( 0x10000L ) );
        static_assert( !is_safe_narrowing_conversion<int32_t>( 0xffffffffUL ) );
    }

    {
        enum class u16_enum : uint16_t
        {};

        enum class i16_enum : int16_t
        {};

        enum class u32_enum : uint32_t
        {};

        enum class i32_enum : int32_t
        {};

        static_assert( is_safe_underlying_conversion_v<uint16_t, u16_enum> );
        static_assert( !is_safe_underlying_conversion_v<int16_t, u16_enum> );
        static_assert( !is_safe_underlying_conversion_v<uint16_t, i16_enum> );
        static_assert( is_safe_underlying_conversion_v<int16_t, i16_enum> );

        static_assert( is_safe_underlying_conversion_v<uint32_t, u16_enum> );
        static_assert( is_safe_underlying_conversion_v<int32_t, u16_enum> );
        static_assert( !is_safe_underlying_conversion_v<uint32_t, i16_enum> );
        static_assert( is_safe_underlying_conversion_v<int32_t, i16_enum> );

        static_assert( !is_safe_underlying_conversion_v<uint16_t, u32_enum> );
        static_assert( !is_safe_underlying_conversion_v<int16_t, u32_enum> );
        static_assert( !is_safe_underlying_conversion_v<uint16_t, i32_enum> );
        static_assert( !is_safe_underlying_conversion_v<int16_t, i32_enum> );

        static_assert( is_safe_underlying_conversion_v<uint32_t, u32_enum> );
        static_assert( !is_safe_underlying_conversion_v<int32_t, u32_enum> );
        static_assert( !is_safe_underlying_conversion_v<uint32_t, i32_enum> );
        static_assert( is_safe_underlying_conversion_v<int32_t, i32_enum> );

        static_assert( is_safe_underlying_conversion_v<uint64_t, u32_enum> );
        static_assert( is_safe_underlying_conversion_v<int64_t, u32_enum> );
        static_assert( !is_safe_underlying_conversion_v<uint64_t, i32_enum> );
        static_assert( is_safe_underlying_conversion_v<int64_t, i32_enum> );

        constexpr auto u16_e = underlying_cast<u16_enum>( uint16_t{ 0 } );
        constexpr auto u32_e = underlying_cast<u32_enum>( u16_e );
        constexpr auto i64 = underlying_cast<int64_t>( u32_e );
        constexpr auto i16 = narrow_cast<int16_t>( i64 );
        constexpr auto i16_e = underlying_cast<i16_enum>( i16 );
        static_assert( !underlying_cast<int16_t>( i16_e ) );
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
        struct test_handle
        {
        public:
            test_handle() noexcept = default;

            test_handle( int right ) noexcept
                : value{ right }
            {
                assert( value != closed_value );
            }

            ~test_handle() noexcept
            {
                if ( check_dtor )
                {
                    check_dtor( value, closed_value );
                }
            }

            void close() noexcept
            {
                assert( check_dtor || check_close );
                assert( closed_value != value );
                closed_value = value;

                if ( check_close )
                {
                    auto temp = std::move( check_close );
                    check_close = {};
                    temp( *this );
                }
            }

            constexpr bool is_valid() const noexcept
            {
                return value && value != closed_value;
            }

            int value = 0;
            int closed_value = -1;
            std::function<void( int, int )> check_dtor;
            std::function<void( test_handle& )> check_close;
        };

#pragma warning( push )
#pragma warning( disable : 26415 ) 
#pragma warning( disable : 26418 )
        constexpr struct
        {
            constexpr bool operator () ( const safe_handle<test_handle>& h1, int value ) const noexcept
            {
                assert( h1.is_unique() && h1.next() == &h1 && h1.previous() == &h1 );
                assert( h1->value == value );
                return true;
            }

            constexpr bool operator () ( const safe_handle<test_handle>& h1, const safe_handle<test_handle>& h2, int value ) const noexcept
            {
                assert( !h1.is_unique() && h1.next() == &h2 && h1.previous() == &h2 );
                assert( !h2.is_unique() && h2.next() == &h1 && h2.previous() == &h1 );
                assert( h1->value == value && h2->value == value );
                return true;
            }

            constexpr bool operator () ( const safe_handle<test_handle>& h1, const safe_handle<test_handle>& h2, const safe_handle<test_handle>& h3, int value ) const noexcept
            {
                assert( !h1.is_unique() && h1.next() == &h2 && h1.previous() == &h3 );
                assert( !h2.is_unique() && h2.next() == &h3 && h2.previous() == &h1 );
                assert( !h3.is_unique() && h3.next() == &h1 && h3.previous() == &h2 );
                assert( h1->value == value && h2->value == value && h3->value == value );
                return true;
            };
        } check;

        constexpr auto unsafe = [] ( const safe_handle<test_handle>& handle ) noexcept -> test_handle &
        {
            return const_cast<test_handle&>( handle.get() );
        };
#pragma warning(pop)


        safe_handle<test_handle> h1{ 1 };
        check( h1, 1 );

        { // self assignment
            h1 = h1;
            check( h1, 1 );
        }

        {   // smart handle closure
            bool h2_was_closed = false;
            {
                safe_handle<test_handle> h2{ 2 };
                unsafe( h2 ).check_dtor = [ &h2_was_closed ] ( int value, int closed_value ) noexcept
                {
                    h2_was_closed = ( value == 2 && closed_value == value );
                };
            }
            assert( h2_was_closed );
        }

        { // copy constructor
            safe_handle<test_handle> h2{ h1 };
            check( h1, h2, 1 );
        }
        check( h1, 1 );

        { // assignment initialization 
            bool h2_was_closed = false;
            safe_handle<test_handle> h2;
            unsafe( h2 ).check_close = [ &h2_was_closed ] ( test_handle& closing_handle ) noexcept
            {
                h2_was_closed = ( closing_handle.value == 0 );
            };
            h2 = h1;
            check( h1, h2, 1 );
            assert( h2_was_closed );
            assert( !( h2->check_dtor ) );
        }
        check( h1, 1 );

        {   // assignment
            safe_handle<test_handle> h2{ 2 };

            bool h1_was_closed = false;
            unsafe( h1 ).check_close = [ &h1_was_closed ] ( test_handle& closing_handle ) noexcept
            {
                h1_was_closed = ( closing_handle.value == 1 );
            };

            h1 = h2;
            check( h1, h2, 2 );
            assert( h1_was_closed );
            assert( !( h1->check_dtor ) );
        }
        check( h1, 2 );
        unsafe( h1 ).value = 1;

        {   // cyclic assignment
            safe_handle<test_handle> h2{ h1 };
            check( h1, h2, 1 );
            h1 = h2;
            check( h1, h2, 1 );
            h2 = h1;
            check( h1, h2, 1 );
        }
        check( h1, 1 );

        {   // cyclic assignment (ref count > 2)
            safe_handle<test_handle> h2{ h1 };
            safe_handle<test_handle> h3{ h2 };
            check( h1, h2, h3, 1 );
            h2 = h3;
            check( h3, h2, h1, 1 );
            h3 = h2;
            check( h2, h3, h1, 1 );
            h1 = h2;
            check( h2, h1, h3, 1 );
        }
        check( h1, 1 );

        {   // assignment (ref count >= 2)
            safe_handle<test_handle> ch1{ h1 };
            check( h1, ch1, 1 );

            safe_handle<test_handle> h2{ 2 };
            safe_handle<test_handle> ch2{ h2 };
            check( h2, ch2, 2 );

            h2 = h1;
            check( ch1, h1, h2, 1 );
            check( ch2, 2 );

            ch1 = ch2;
            check( ch1, ch2, 2 );
            check( h1, h2, 1 );

            h2 = ch2;
            check( ch1, ch2, h2, 2 );
            check( h1, 1 );

            ch1 = h1;
            check( h2, ch2, 2 );
            check( h1, ch1, 1 );

            unsafe( h2 ).check_dtor = [] ( int value, int closed_value ) noexcept
            {
                assert( value == 2 && closed_value == value );
            };
        }

        check( h1, 1 );
        unsafe( h1 ).check_dtor = [] ( int value, int closed_value ) noexcept
        {
            assert( value == 1 && closed_value == value );
        };
    }

    {
        using map_ii7 = static_map<int, int, 7>;
        map_ii7 map;

        static_assert( std::is_trivial_v<typename map_ii7::value_type> );

        auto check_contains = [&map = std::as_const( map )]( bool contains, int key, int value )
        {
            assert( std::is_sorted( map.cbegin(), map.cend(), map.less ) );
            assert( std::adjacent_find( map.cbegin(), map.cend(), map.eq ) == map.cend() );

            if ( map.is_static() )
            {
                assert( map.capacity() == map.static_size );
            }
            assert( map.size() <= map.capacity() );

            const auto size = map.size();
            assert( map.cbegin() == map.data() );
            assert( map.cend() == map.data() + size );

            assert( map.contains( key ) == contains );

            const auto item = map.item( key );
            assert( item.has_value == contains );
            const auto position = item.position;

            const auto find_it = map.find( key );
            const auto items = map.items( key );
            const auto count = map.count( key );

            if ( contains )
            {
                assert( size > 0_z );
                assert( position >= map.cbegin() && position < map.cend() );
                assert( position->key == key && position->value == value );
                assert( find_it == position );
                assert( items.data() == position && items.size() == 1_z );
                assert( count == 1_z );
            }
            else
            {
                assert( position >= map.cbegin() && position <= map.cend() );
                assert( find_it == map.cend() );
                assert( items.data() == position && items.size() == 0_z );
                assert( count == 0_z );
            }

            return position;
        };

        auto check_insert = [&map, &check_contains]( int key, int value )
        {
            const auto size = map.size();
            const auto capacity = map.capacity();
            const auto is_static = map.is_static();
            const auto data = map.data();

            check_contains( false, key, value );
            const auto ins = map.insert( key, value );
            assert( ins.second );
            const auto pos = check_contains( true, key, value );
            assert( ins.first == pos );
            assert( map.size() == size + 1_z );

            if ( is_static != map.is_static() )
            {
                assert( is_static );
                assert( size == map.static_size );
                assert( map.data() != data );
                assert( map.capacity() > capacity );
            }
            else
            {
                if ( is_static )
                {
                    assert( map.data() == data );
                    assert( map.capacity() == capacity );
                }
                else
                {
                    if ( map.data() != data )
                    {
                        assert( map.capacity() > capacity );
                    }
                    else
                    {
                        assert( map.capacity() == capacity );
                    }
                }
            }

            return pos;
        };

        auto check_stabile = [&map = std::as_const( map )]( std::function<map_ii7::const_iterator ( map_ii7& )> noise )
        {
            const auto size = map.size();
            const auto is_static = map.is_static();
            const auto data = map.data();
            const std::vector< map_ii7::value_type > copy{ map.cbegin(), map.cend() };

            const auto result = noise( const_cast<map_ii7&>( map ) );

            assert( size == map.size() );
            assert( is_static == map.is_static() );
            assert( data == map.data() );
            assert( std::equal( copy.cbegin(), copy.cend(), map.cbegin() ) );

            return result;
        };

        auto check_insert_fail = [ &check_stabile, &check_contains ]( int key, int value, int true_value )
        {
            return check_stabile( [&check_contains, key, value, true_value] ( map_ii7& map )
            {
                const auto pos = check_contains( true, key, true_value );
                const auto ins = map.insert( key, value );
                assert( !ins.second );
                assert( ins.first == pos );
                assert( ins.first->value == true_value );
                assert( check_contains( true, key, true_value ) == pos );
                return pos;
            } );
        };

        auto check_erase_fail = [ &check_stabile, &check_contains ] ( int key )
        {
            return check_stabile( [&check_contains, key] ( map_ii7& map )
            {
                check_contains( false, key, 0 );
                map.erase( key );
                return map.cbegin();
            } );
        };

        check_insert( 1, 1 );
        check_insert( 4, 16 );
        check_insert( 2, 4 );
        check_insert_fail( 2, 5, 4 );
        check_insert( 3, 9 );
        check_insert( 5, 25 );
        check_insert_fail( 5, 26, 25 );
        check_insert_fail( 3, 8, 9 );
        check_insert( 8, 64 );
        check_insert( 7, 49 );

        {
            assert( map.is_static() && map.size() == map.static_size );
            check_insert( 6, 36 );
            assert( !map.is_static() && map.size() == map.static_size + 1_z );
        }

        auto check_erase_back = [&map, &check_contains]( int key, int value, int back_key, int back_value, int front_key, int front_value )
        {
            assert( map.size() >= 3 );
            assert( key > back_key );
            assert( back_key > front_key );

            const auto size = map.size();
            const auto is_static = map.is_static();
            const auto data = map.data();

            assert( check_contains( true, key, value ) == std::prev( map.cend() ) );
            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend(), 2 ) );
            assert( check_contains( true, front_key, front_value ) == map.cbegin() );

            map.erase( key );

            assert( check_contains( true, front_key, front_value ) == map.cbegin() );
            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            check_contains( false, key, value );

            assert( map.cend()->key == key && map.cend()->value == value );

            assert( map.data() == data );
            assert( map.is_static() == is_static );
            assert( map.size() == size - 1_z );
        };

        auto check_erase_front = [&map, &check_contains]( int key, int value, int front_key, int front_value, int back_key, int back_value )
        {
            assert( map.size() >= 3 );
            assert( key < front_key );
            assert( front_key < back_key );

            const auto size = map.size();
            const auto is_static = map.is_static();
            const auto data = map.data();

            assert( check_contains( true, key, value ) == map.cbegin() );
            assert( check_contains( true, front_key, front_value ) == std::next( map.cbegin() ) );
            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );

            map.erase( key );

            assert( check_contains( true, front_key, front_value ) == map.cbegin() );
            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            check_contains( false, key, value );

            assert( map.cend()->key == back_key && map.cend()->value == back_value );

            assert( map.data() == data );
            assert( map.is_static() == is_static );
            assert( map.size() == size - 1_z );
        };

        auto check_erase_preback = [&map, &check_contains]( int key, int value, int back_key, int back_value, int preback_key, int preback_value, int front_key, int front_value )
        {
            assert( map.size() >= 4 );
            assert( back_key > key );
            assert( key > preback_key );
            assert( preback_key > front_key );

            const auto size = map.size();
            const auto is_static = map.is_static();
            const auto data = map.data();

            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            assert( check_contains( true, key, value ) == std::prev( map.cend(), 2 ) );
            assert( check_contains( true, preback_key, preback_value ) == std::prev( map.cend(), 3 ) );
            assert( check_contains( true, front_key, front_value ) == map.cbegin() );

            map.erase( key );

            assert( check_contains( true, front_key, front_value ) == map.cbegin() );
            assert( check_contains( true, preback_key, preback_value ) == std::prev( map.cend(), 2 ) );
            check_contains( false, key, value );
            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );

            assert( map.cend()->key == back_key && map.cend()->value == back_value );

            assert( map.data() == data );
            assert( map.is_static() == is_static );
            assert( map.size() == size - 1_z );
        };

        auto check_erase_prepreback = [&map, &check_contains](
            int key, int value, 
            int back_key, int back_value, 
            int preback_key, int preback_value,
            int prepreback_key, int prepreback_value, 
            int front_key, int front_value )
        {
            assert( map.size() >= 5 );
            assert( back_key > preback_key );
            assert( preback_key > key );
            assert( key > prepreback_key );
            assert( prepreback_key > front_key );

            const auto size = map.size();
            const auto is_static = map.is_static();
            const auto data = map.data();

            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            assert( check_contains( true, preback_key, preback_value ) == std::prev( map.cend(), 2 ) );
            assert( check_contains( true, key, value ) == std::prev( map.cend(), 3 ) );
            assert( check_contains( true, prepreback_key, prepreback_value ) == std::prev( map.cend(), 4 ) );
            assert( check_contains( true, front_key, front_value ) == map.cbegin() );

            map.erase( key );

            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            assert( check_contains( true, preback_key, preback_value ) == std::prev( map.cend(), 2 ) );
            check_contains( false, key, value );
            assert( check_contains( true, prepreback_key, prepreback_value ) == std::prev( map.cend(), 3 ) );
            assert( check_contains( true, front_key, front_value ) == map.cbegin() );

            assert( map.cend()->key == back_key && map.cend()->value == back_value );

            assert( map.data() == data );
            assert( map.is_static() == is_static );
            assert( map.size() == size - 1_z );
        };

        auto check_shrink_to_static = [&map, &check_contains]( int back_key, int back_value, int front_key, int front_value )
        {
            assert( map.size() == map.static_size );
            assert( map.capacity() > map.static_size );
            assert( !map.is_static() );
            assert( back_key > front_key );

            const auto size = map.size();
            const auto data = map.data();
            const std::vector<map_ii7::value_type > copy( map.cbegin(), map.cend() );

            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            assert( check_contains( true, front_key, front_value ) == map.cbegin() );

            map.shrink_to_fit();

            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            assert( check_contains( true, front_key, front_value ) == map.cbegin() );

            assert( map.data() != data );
            assert( map.is_static() );
            assert( map.capacity() == map.static_size );
            assert( map.size() == size );
            assert( std::equal( copy.cbegin(), copy.cend(), map.cbegin() ) );
        };

        auto check_shrink_to_fit_dynamic = [ &map, &check_contains]( int back_key, int back_value, int front_key, int front_value )
        {
            assert( map.size() > map.static_size );
            assert( map.capacity() > map.size() );
            assert( !map.is_static() );
            assert( back_key > front_key );

            const auto size = map.size();
            const auto is_static = map.is_static();

            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            assert( check_contains( true, front_key, front_value ) == map.cbegin() );

            map.shrink_to_fit();

            assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
            assert( check_contains( true, front_key, front_value ) == map.cbegin() );

            assert( map.is_static() == is_static );
            assert( map.capacity() == map.size() );
            assert( map.size() == size );
        };

        {
            assert( !map.is_static() && map.size() == map.static_size + 1_z );
            check_erase_back( 8, 64, 7, 49, 1, 1 );
            check_shrink_to_static( 7, 49, 1, 1 );
            check_insert( 8, 64 );
        }

        {
            assert( !map.is_static() && map.size() == map.static_size + 1_z );
            check_erase_preback( 7, 49, 8, 64, 6, 36, 1, 1 );
            check_shrink_to_static( 8, 64, 1, 1 );
            check_insert( 7, 49 );
        }

        {
            assert( !map.is_static() && map.size() == map.static_size + 1_z );
            check_erase_front( 1, 1, 2, 4, 8, 64 );
            check_shrink_to_static( 8, 64, 2, 4 );
            check_insert( 1, 1 );
        }

        {
            assert( !map.is_static() && map.size() == map.static_size + 1_z );
            check_insert( 9, 81 );
            check_erase_prepreback( 7, 49, 9, 81, 8, 64, 6, 36, 1, 1 );
            check_shrink_to_fit_dynamic( 9, 81, 1, 1 );
        }

        {
            assert( !map.is_static() && map.size() == map.static_size + 1_z );
            check_erase_fail( -1 );
            check_erase_fail( 10 );
            check_erase_fail( 7 );
        }

        {
            assert( !map.is_static() && map.size() == map.static_size + 1_z );
            check_erase_prepreback( 6, 36, 9, 81, 8, 64, 5, 25, 1, 1 );
            check_shrink_to_static( 9, 81, 1, 1 );
            check_erase_prepreback( 5, 25, 9, 81, 8, 64, 4, 16, 1, 1 );
            check_erase_front( 1, 1, 2, 4, 9, 81 );
            check_erase_back( 9, 81, 8, 64, 2, 4 );
            check_erase_fail( 1 );
            check_erase_fail( 9 );
            check_erase_fail( 5 );
            assert( map.is_static() );
        }

        {
            assert( map.count( 4 ) == 1_z );
            map.force_insert( 4, 17 );
            map.force_insert( 4, 18 );
            map.force_insert( 4, 19 );
            const auto items = map.items( 4 );
            constexpr map_ii7::value_type check[]
            {
                { 4, 16 },
                { 4, 17 },
                { 4, 18 },
                { 4, 19 }
            };
            assert( items.size() == std::size( check ) );
            assert( std::equal( items.begin(), items.end(), check ) );
            assert( map.erase( 4 ) == std::size( check ) );
            check_contains( false, 4, 16 );
        }

        {
            struct checker
            {
                static std::set<int>& for_destroy() noexcept
                {
                    static std::set<int> for_destroy_;
                    return for_destroy_;
                }

                static int unique_id() noexcept
                {
                    static int id = 0;
                    return ++id;
                }

                int i{ 0 };
                mutable int id = 0;

                checker() = default;

                checker( int i ) noexcept
                    : i{ i }
                {}

                ~checker()
                {
                    if ( id )
                    {
                        assert( for_destroy().erase( id ) == 1_z );
                    }
                }

                checker( const checker& right ) noexcept
                    : i{ right.i }
                {
                    assert( true );
                }

                checker& operator = ( const checker& ) noexcept
                {
                    assert( true );
                    return *this;
                }

                checker( checker&& right ) noexcept
                    : i{ right.release_i() }
                    , id{ right.release_id() }
                {}

                checker& operator = ( checker&& right ) noexcept
                {
                    std::swap( i, right.i );
                    std::swap( id, right.id );
                    return *this;
                }

                bool operator < ( const checker& right ) const noexcept
                {
                    return i < right.i;
                }

                bool operator == ( const checker& right ) const noexcept
                {
                    return i == right.i;
                }

                int release_i() noexcept
                {
                    int temp = i;
                    i = 0;
                    return temp;
                }

                int release_id() noexcept
                {
                    int temp = id;
                    id = 0;
                    return temp;
                }

                void enable_check_destroy() const
                {
                    assert( !id );
                    id = unique_id();
                    assert( for_destroy().insert( id ).second );
                }

                int check_enable_check_destroy() const
                {
                    assert( id );
                    assert( for_destroy().contains( id ) );
                    return id;
                }
            };

            static_map<checker, checker, 4> checker_map;
            auto insert = [ &checker_map ] ( int key, int value )
            {
                const auto ins = checker_map.insert( key, value );
                assert( ins.second );
                ins.first->key.enable_check_destroy();
                ins.first->value.enable_check_destroy();
            };

            auto erase = [ &checker_map ] ( int key )
            {
                const auto items = checker_map.items( key );
                assert( items.size() == 1_z );
                const auto size = checker::for_destroy().size();
                const auto key_id = items.front().key.check_enable_check_destroy();
                const auto value_id = items.front().value.check_enable_check_destroy();
                checker_map.erase( key );
                assert( checker::for_destroy().size() == size - 2_z );
                assert( !checker::for_destroy().contains( key_id ) );
                assert( !checker::for_destroy().contains( value_id ) );
            };

            insert( 1, 1 );
            insert( 4, 16 );
            insert( 2, 4 );
            assert( !checker_map.insert( 2, 5 ).second );
            insert( 6, 36 );
            insert( 10, 100 );
            insert( 7, 49 );
            erase( 7 );
            erase( 6 );
            checker_map.erase( 5 );
            erase( 4 );
            checker_map.erase( 3 );
            erase( 2 );
            erase( 1 );
            erase( 10 );
        }
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
