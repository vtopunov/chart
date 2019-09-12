#include <core/rational.h>
#include <core/assert.h>

void test_rational() noexcept
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

    assert( !errno );
}