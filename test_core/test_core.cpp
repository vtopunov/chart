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

            void close( const void* ) noexcept
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

            void construct_weak( const void* ) noexcept {}

            void replace_weak( const void*, const void* ) noexcept {}

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

        constexpr auto unsafe = [] ( const safe_handle<test_handle>& handle ) -> test_handle&
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
                unsafe( h2 ).check_dtor = [ &h2_was_closed ] ( int value, int closed_value )
                {
                    h2_was_closed  = ( value == 2 && closed_value == value );
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
            unsafe( h2 ).check_close = [ &h2_was_closed ] ( test_handle& closing_handle )
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
            unsafe( h1 ).check_close = [ &h1_was_closed ] ( test_handle& closing_handle )
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

            unsafe( h2 ).check_dtor = [] ( int value, int closed_value )
            {
                assert( value == 2 && closed_value == value );
            };
        }

        check( h1, 1 );
        unsafe( h1 ).check_dtor = [] ( int value, int closed_value )
        {
            assert( value == 1 && closed_value == value );
        };
    }

    {
        static_map<int, int, 7> map;

        auto check = [ &map ] ()
        {
            return std::is_sorted( map.cbegin(), map.cend(), map.less ) && std::adjacent_find( map.cbegin(), map.cend(), map.eq ) == map.cend();
        };

        auto exist = [ &map ] ( int key, int value ) -> bool
        {
            {
                const auto it = std::as_const( map ).find( key );
                if ( it == std::as_const( map ).end() || it->key != key || it->value != value )
                {
                    return false;
                }
            }

            {
                const auto it = std::as_const( map ).item( key );
                if ( !it || it == std::as_const( map ).end() || it->key != key || it->value != value )
                {
                    return false;
                }
            }

            return true;
        };

        assert( map.insert( 1, 1 ).second && check() );
        assert( map.insert( 4, 16 ).second && check() );
        assert( map.insert( 2, 4 ).second && check() );
        assert( !map.insert( 2, 5 ).second && check() );
        assert( map.insert( 3, 9 ).second && check() );
        assert( map.insert( 5, 25 ).second && check() );

        assert( map.is_static() && map.size() == 5 && check() );
        assert( exist( 1, 1 ) && check() );
        assert( exist( 2, 4 ) && check() );
        assert( exist( 3, 9 ) && check() );
        assert( exist( 4, 16 ) && check() );
        assert( exist( 5, 25 ) && check() );
        assert( !map.insert( 5, 25 ).second && check() );
        assert( !map.insert( 3, 9 ).second && check() );
        
        {
            assert( map.find( 3 )->value == 9 && check() );
            const auto result = as_const_pointer( map.insert_or_assign( 3, 8 ) );
            assert( result->key == 3 && result->value == 8 && exist( 3, 8 ) && map.is_static() && check() );
        }

        assert( map.insert( 8, 64 ).second && check() );
        assert( map.insert( 7, 49 ).second && check() );
        assert( map.is_static() && check() );
        assert( map.insert( 6, 36 ).second && check() );
        assert( !map.is_static() && check() );

        assert( exist( 6, 36 ) && check() );
        assert( exist( 7, 49 ) && check() );
        assert( exist( 8, 64 ) && check() );

        {
            assert( exist( 3, 8 ) && check() );
            const auto result = as_const_pointer( map.insert_or_assign( 3, 9 ) );
            assert( result->key == 3 && result->value == 9 && exist( 3, 9 ) && !map.is_static()  && check() );
        }

        assert( !map.is_static() && map.item( 8 ) && check() );
        map.erase( 8 );
        assert( map.is_static() && !map.item( 8 ) && check() );
        map.insert( 8, 64 );

        assert( !map.is_static() && map.item( 7 ) && check() );
        map.erase( 7 );
        assert( map.is_static() && !map.item( 7 ) && check() );
        map.insert( 7, 49 );

        assert( !map.is_static() && map.item( 1 ) && check() );
        map.erase( 1 );
        assert( map.is_static() && !map.item( 1 ) && check() );
        map.insert( 1, 1 );

        assert( !map.is_static() && check() );
        map.insert( 9, 81 );
        map.erase( 7 );
        assert( !map.is_static() && !map.item( 7 ) && check() );

        {
            assert( !map.is_static() && check() );

            const auto size = map.size();
            map.erase( -1 );
            assert( size == map.size() && check() );

            map.erase( 10 );
            assert( size == map.size() && check() );

            map.erase( 7 );
            assert( size == map.size() && check() );
        }


        assert( !map.is_static() && map.item( 3 ) && check() );
        map.erase( 3 );
        assert( map.is_static() && !map.item( 3 ) && check() );

        assert( map.is_static() && map.item( 5 ) && check() );
        map.erase( 5 );
        assert( map.is_static() && !map.item( 5 ) && check() );

        assert( map.is_static() && map.item( 1 ) && check() );
        map.erase( 1 );
        assert( map.is_static() && !map.item( 1 ) && check() );

        assert( map.is_static() && map.item( 9 ) && check() );
        map.erase( 9 );
        assert( map.is_static() && !map.item( 9 ) && check() );


        {
            assert( map.is_static() && !map.item( 1 ) && !map.item( 9 ) && !map.item( 5 ) && check() );

            const auto size = map.size();

            map.erase( 1 );
            assert( size == map.size() && check() );

            map.erase( 9 );
            assert( size == map.size() && check() );

            map.erase( 5 );
            assert( size == map.size() && check() );
        }

        struct move_check
        {
            int i{ 0 };
            
            move_check() = default;

            move_check( int i ) noexcept 
                : i{ i } 
            {}

            move_check( const move_check& right ) noexcept
                : i{ right.i }
            { 
                assert( true ); 
            }

            move_check& operator = ( const move_check& right ) noexcept
            {
                i = right.i;
                assert( true );
                return *this;
            }

            move_check( move_check&& right ) noexcept 
                : i{ right.release() } 
            {}

            move_check& operator = ( move_check&& right ) noexcept 
            { 
                std::swap( i, right.i ); 
                return *this;
            }

            bool operator < ( const move_check& right ) const noexcept
            {
                return i < right.i;
            }

            bool operator == ( const move_check& right ) const noexcept
            {
                return i == right.i;
            }

            int release() noexcept
            {
                int temp = i;
                i = 0;
                return temp;
            }
        };

        static_map<move_check, move_check, 4> move;
        assert( move.insert( 1, 1 ).second );
        assert( move.insert( 4, 16 ).second );
        assert( move.insert( 2, 4 ).second );
        assert( !move.insert( 2, 5 ).second );
        assert( move.insert( 6, 36 ).second );
        assert( move.insert( 5, 25 ).second );
        assert( move.insert( 10, 100 ).second );
        assert( move.insert( 7, 49 ).second );
        assert( move.insert_or_assign( 7, 49 )->value.i == 49 );
        move.erase( 7 );
        move.erase( 6 );
        move.erase( 5 );
        move.erase( 4 );
        move.erase( 3 );
        move.erase( 2 );
        move.erase( 1 );
        move.erase( 10 );
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
