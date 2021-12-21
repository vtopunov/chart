#include <core/rational.h>
#include <core/assert.h>

void test_rational() noexcept
{
    using namespace rational_literals;

    static_assert(std::is_trivial_v<rational<int>> && std::is_standard_layout_v<rational<int>>);

    static_assert(is_rational_v< rational<int>>);
    static_assert(is_rational_v< const rational<int> >);
    static_assert(!is_rational_v< int >);
    static_assert(!is_rational_v< std::span<int> >);
    static_assert(!is_rational_v< std::span<int, 2_uz> >);
    static_assert(!is_rational_v< std::span<const int> >);
    static_assert(!is_rational_v< std::span<const int, 2_uz> >);
    static_assert(sizeof(rational<char>) == 2);

    constexpr rational r0{ 2, 3 };
    constexpr rational r1{ 3, 4 };
    constexpr rational r2{ 8, 10 };
    constexpr auto r3 = simplify(r2);

    static_assert(r3 == rational{ 4, 5 });
    static_assert(-r3 == rational{ -4, 5 });
    static_assert(r0 + r1 == rational{ 17, 12 });
    static_assert(r1 + r0 == r0 + r1);
    static_assert(r0 - r1 == rational{ -1, 12 });
    static_assert(r1 - r0 == rational{ 1, 12 });
    static_assert(-r0 + r1 == r1 - r0);
    static_assert(r0 * r1 == rational{ 1, 2 });
    static_assert(r1 * r0 == r0 * r1);
    static_assert(r0 / r1 == rational{ 8, 9 });
    static_assert((r0 * r1) / r1 == r0);
    static_assert(r0 * 3 == rational{ 2, 1 });
    static_assert(2 * r1 == rational{ 3, 2 });
    static_assert(3 / rational{ 2, 5 } == rational{ 15, 2 });
    static_assert((1 / 2_r) == rational<ptrdiff_t>{ 1, 2 });
    static_assert(0.0_r == rational<ptrdiff_t>::zero());
    static_assert(std::is_same_v<decltype(0.0_r), rational<ptrdiff_t>>);
    static_assert(0.5_r == rational<ptrdiff_t>{ 1, 2 });
    static_assert(1.5_r == rational<ptrdiff_t>{ 3, 2 });
    static_assert(-1.5_r == rational<ptrdiff_t>{ -3, 2 });
    static_assert(1.5_ur == rational<size_t>{ 3, 2 });
    static_assert(std::is_same_v<decltype(0.0_ur), rational<size_t>>);
    static_assert(rational_cast<rational<intmax_t>>(rational{ '\x12', '\x34' }) == rational<intmax_t>{0x12, 0x34});
    static_assert(rational_cast<rational<int>>(2) == rational{ 2, 1 });
    static_assert(rational_cast<int>(rational{ 3, 2 }) == 1);
    static_assert(rational_cast<int>(rational{ 1, 2 }) == 0);
    static_assert(rational_cast<int>(rational{ 13, 7 }) == 1);
    static_assert(rational_cast<int>(rational{ 14, 7 }) == 2);

    {
        constexpr rational ir{ 2, 3 };
        constexpr rational uir{ 4u, 5u };

        constexpr rational<intmax_t> rmaxir = ir;
        static_assert(rmaxir.num == ir.num);
        static_assert(rmaxir.den == ir.den);

        constexpr rational<intmax_t> rmaxuir = uir;
        static_assert(rmaxuir.num == uir.num);
        static_assert(rmaxuir.den == uir.den);
    }

    D_ASSERT(!errno);
}