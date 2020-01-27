#include <core/rational.h>
#include <core/assert.h>

void test_rational() noexcept
{
    constexpr rational r0{ 2, 3 };
    constexpr rational r1{ 3, 4 };
    constexpr rational r2{ 8, 10 };
    constexpr rational r3 = simplify( r2 );
    static_assert(r3 == rational{ 4, 5 });
    static_assert(-r3 == rational{ -4, 5 });
    static_assert(r0 + r1 == rational{ 17, 12 });
    static_assert( r1 + r0 == r0 + r1 );
    static_assert(r0 - r1 == rational{ -1, 12 });
    static_assert(r1 - r0 == rational{ 1, 12 });
    static_assert( -r0 + r1 == r1 - r0 );
    static_assert(r0 * r1 == rational{ 1, 2 });
    static_assert( r1 * r0 == r0 * r1 );
    static_assert(r0 / r1 == rational{ 8, 9 });
    static_assert( ( r0 * r1 ) / r1 == r0 );

    D_ASSERT( !errno );
}