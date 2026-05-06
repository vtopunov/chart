#include <cmath>
#include <string_view>

#include <core/rational.h>


void test_rational() noexcept
{
    const errno_holder hold_errno{};

    using namespace rational_literals;

    static_assert(std::is_trivial_v<rational<int>> && std::is_standard_layout_v<rational<int>>);

    static_assert(is_rational_v< rational<int>>);
    static_assert(is_rational_v< const rational<int> >);
    static_assert(!is_rational_v< int >);
    static_assert(!is_rational_v< span<int> >);
    static_assert(!is_rational_v< span<int, 2_uz> >);
    static_assert(!is_rational_v< span<const int> >);
    static_assert(!is_rational_v< span<const int, 2_uz> >);
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
    static_assert(rational<int>::instance(5) == rational{ 5, 1 });
    static_assert(rational<ptrdiff_t>::zero() == rational<ptrdiff_t>::instance(0));
    static_assert(rational<ptrdiff_t>::zero() == rational<ptrdiff_t>{ 0, 1 });
    static_assert(rational<ptrdiff_t>::zero() == zero_v<rational<ptrdiff_t>>);
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

    {
        constexpr int den = 8;
        static_assert(std::has_single_bit(to_unsigned(den)));
        constexpr int fraction_width = std::bit_width(to_unsigned(den)) - 1;
        constexpr int mask = den - 1;
        constexpr double denf{ den };
        using fixed3bit_t = rational<int, den>;
        
        {
            constexpr auto r_0_125 = fixed3bit_t::instance(std::string_view{ "0.125" });
            constexpr auto r_37_125 = fixed3bit_t::instance(std::string_view{ "37.125" });
            constexpr auto r_0_25 = fixed3bit_t::instance(std::string_view{ "0.25" });
            constexpr auto r_37_25 = fixed3bit_t::instance(std::string_view{ "37.25" });
            static_assert(1 == r_0_125.num);
            static_assert(37 * den + 1 == r_37_125.num);
            static_assert(2 == r_0_25.num);
            static_assert(37 * den + 2 == r_37_25.num);
        }

        for (int i = -6; i < 6; ++i)
        {
            for (int j = 0; j < den; ++j)
            {
                const auto num = (i * den + j);
                const auto discard_fraction = (num >> fraction_width);
                const auto fraction = (num & mask);
                const auto discard_fraction2 = num / den;
                const auto fraction2 = num % den;
                const auto ff = discard_fraction + fraction / denf;
                const auto ff2 = discard_fraction2 + fraction2 / denf;
                const auto ff3 = num / denf;
                D_ASSERT(is_eqfp(ff, ff2));
                D_ASSERT(is_eqfp(ff, ff3));


                fixed3bit_t fx{ i * den + j };
                rational<int> xz{ fx.num, fx.den };

                static_assert(den == fx.den);
                static_assert(fraction_width == decltype(fx)::denominator_traits_type::fraction_width);
                D_ASSERT(fx.discard_fraction() == i);
                D_ASSERT(fx.fraction() == j);
                D_ASSERT(xz.discard_fraction() == discard_fraction2);
                D_ASSERT(xz.fraction() == fraction2);

                const fixed3bit_t ppfx{ fx.num + fx.den };
                const fixed3bit_t mmfx{ fx.num - fx.den };
                D_ASSERT((fx + 1) > fx);
                D_ASSERT((fx - 1) < fx);
                D_ASSERT((fx + 1) == ppfx);
                D_ASSERT((fx - 1) == mmfx);
                D_ASSERT((fx + 1) != fx);
                D_ASSERT((fx - 1) != fx);

                D_ASSERT((fx + 1) >= fx);
                D_ASSERT((fx - 1) <= fx);
                D_ASSERT((fx + 1) >= ppfx);
                D_ASSERT((fx + 1) <= ppfx);
                D_ASSERT((fx + 1) >= mmfx);
                D_ASSERT((fx - 1) >= mmfx);
                D_ASSERT((fx - 1) <= mmfx);
                D_ASSERT((fx - 1) <= ppfx);

                const auto ceil_ff = std::ceil(ff);
                const auto floor_ff = std::floor(ff);
                
                D_ASSERT(::is_eqfp(ceil_ff, ceil_to<int>(fx)));
                D_ASSERT(::is_eqfp(floor_ff, floor_to<int>(fx)));
            }
        }
    }
}