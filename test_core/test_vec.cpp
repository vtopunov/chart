#include <core/vec.h>
#include <core/assert.h>

void test_vec() noexcept
{
    constexpr auto v0 = make_vec( 1, 1 );
    constexpr auto v1 = make_vec( 1, 2 );
    constexpr auto v2 = make_vec( 2, 1 );
    constexpr auto v3 = make_vec( 2, 2 );
    constexpr auto v4 = make_vec( 1, 0 );
    constexpr auto v5 = make_vec( 3, 1 );
              
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

    static_assert( reverse(v1) == v2 );
    static_assert( reverse(v2) == v1 );
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

    D_ASSERT( !errno );
}