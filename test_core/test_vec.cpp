#include <core/vec.h>
#include <core/assert.h>

void test_vec() noexcept
{
    constexpr vec v0{ 1, 1 };
    constexpr vec v1{ 1, 2 };
    constexpr vec v2{ 2, 1 };
    constexpr vec v3{ 2, 2 };
    constexpr vec v4{ 1, 0 };
    constexpr vec v5{ 3, 1 };

    static_assert( std::is_trivial_v<vec<int>> && std::is_standard_layout_v<vec<int>> );

    static_assert( v0 != v1 );
    static_assert( max( v1, v2 ) == v3 );
    static_assert( min( v4, v2 ) == v4 );
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