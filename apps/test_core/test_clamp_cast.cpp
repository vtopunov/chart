#include <core/clamp_cast.h>

#include <cerrno>
#include <cinttypes>


namespace
{

    template<class Target, class Source>
    void test_clamp_cast_for() noexcept
    {
        constexpr Source s_min{ numeric_min_v<Source> };
        constexpr Source s_max{ numeric_max_v<Source> };
        constexpr Source s_one{ 1 };

        constexpr Target t_min{ numeric_min_v<Target> };
        constexpr Target t_zero{ 0 };
        constexpr Target t_one{ 1 };
        constexpr Target t_max{ numeric_max_v<Target> };

        constexpr auto t_s_min = static_cast<Target>(s_min);
        constexpr auto t_s_max = static_cast<Target>(s_max);

        constexpr Target r_min
        {
            (std::is_unsigned_v<Target> || std::is_unsigned_v<Source>)
            ? t_zero
            : ((sizeof(Source) < sizeof(Target)) ? t_s_min : t_min)
        };

        constexpr Target r_max{ (numeric_digits_v<Source> < numeric_digits_v<Target>) ? t_s_max : t_max };

        static_assert(r_min >= t_min);
        static_assert(r_min < t_one);
        static_assert(r_max > t_one);
        static_assert(r_max <= t_max);

        static_assert(t_min == clamp_cast<Target>(t_min));
        static_assert(t_one == clamp_cast<Target>(t_one));
        static_assert(t_max == clamp_cast<Target>(t_max));

        static_assert(r_min == clamp_cast<Target>(s_min));
        static_assert(t_one == clamp_cast<Target>(s_one));
        static_assert(r_max == clamp_cast<Target>(s_max));

        D_ASSERT(!errno);
    }

    template<class Int, class Float>
    void test_clamp_cast_for_f() noexcept
    {
        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_truncation_of_value);
        D_WARNING_DISABLE_MSVC(W_arithmetic_overflow);

        static_assert(std::is_integral_v<Int>);
        static_assert(std::is_floating_point_v<Float>);
        constexpr bool is_trunc = numeric_digits_v<Float> < numeric_digits_v<Int>;
        constexpr Int i_zero{ 0 };

        D_ASSERT(i_zero == clamp_cast<Int>(-0.9));
        D_ASSERT(i_zero == clamp_cast<Int>(0.0));
        D_ASSERT(i_zero == clamp_cast<Int>(0.9));

        {
            constexpr auto max_i = numeric_max_v<Int>;
            constexpr auto max_f = static_cast<Float>(max_i);
            const volatile auto i_max_f = static_cast<Int>(max_f);
            D_ASSERT(max_i == clamp_cast<Int>(max_i));
            D_ASSERT(max_f == clamp_cast<Float>(max_f));

            const auto clampi_max_f = clamp_cast<Int>(max_f);
            const auto clampf_max_i = clamp_cast<Float>(max_i);

            const volatile auto f_clampi_max_f = static_cast<Float>(clampi_max_f);
            const volatile auto i_f_clampi_max_f = static_cast<Int>(f_clampi_max_f);
            const auto clampi_inf_f = clamp_cast<Int>(numeric_inf_v<Float>);
            const auto clampi_max_ff = clamp_cast<Int>(max_f + max_f);

            const volatile auto i_clampf_max_i = static_cast<Int>(clampf_max_i);
            const volatile auto f_i_clampf_max_i = static_cast<Float>(i_clampf_max_i);
            const volatile auto ii_clampf_max_i = (is_trunc) ? static_cast<Int>(i_clampf_max_i + i_clampf_max_i) : i_clampf_max_i;
            const auto clampf_ii_clampf_max_i = clamp_cast<Float>(ii_clampf_max_i);

            D_ASSERT(clampi_max_f == i_f_clampi_max_f);
            D_ASSERT(clampi_max_f == clampi_inf_f);
            D_ASSERT(clampi_max_f == clampi_max_ff);

            D_ASSERT(clampf_max_i == f_i_clampf_max_i);
            D_ASSERT(clampf_max_i == clampf_ii_clampf_max_i);

            if (is_trunc)
            {
                D_ASSERT(i_max_f != max_i);
                D_ASSERT(clampi_max_f < max_i);
                D_ASSERT(clampf_max_i < max_f);
                D_ASSERT(i_clampf_max_i < clampi_max_f);
                D_ASSERT(clampf_max_i < f_clampi_max_f);
            }
            else
            {
                D_ASSERT(i_max_f == max_i);
                D_ASSERT(clampi_max_f == max_i);
                D_ASSERT(clampf_max_i == max_f);
                D_ASSERT(clampi_max_f == i_clampf_max_i);
                D_ASSERT(clampf_max_i == f_clampi_max_f);
            }
        }

        {
            constexpr auto min_i = numeric_min_v<Int>;
            constexpr auto min_f = static_cast<Float>(min_i);
            const volatile auto i_min_f = static_cast<Int>(min_f);
            D_ASSERT(min_i == clamp_cast<Int>(min_i));
            D_ASSERT(min_f == clamp_cast<Float>(min_f));

            const auto clampi_min_f = clamp_cast<Int>(min_f);
            const auto clampf_min_i = clamp_cast<Float>(min_i);

            const volatile auto f_clampi_min_f = static_cast<Float>(clampi_min_f);
            const volatile auto i_f_clampi_min_f = static_cast<Int>(f_clampi_min_f);
            const auto clampi_neg_inf_f = clamp_cast<Int>(-numeric_inf_v<Float>);
            const auto clampi_min_ff = clamp_cast<Int>(min_f + min_f);

            const volatile auto i_clampf_min_i = static_cast<Int>(clampf_min_i);
            const volatile auto f_i_clampf_min_i = static_cast<Float>(i_clampf_min_i);
            const volatile auto ii_clampf_min_i = (is_trunc) ? static_cast<Int>(i_clampf_min_i + i_clampf_min_i) : i_clampf_min_i;
            const auto clampf_ii_clampf_min_i = clamp_cast<Float>(ii_clampf_min_i);

            D_ASSERT(clampi_min_f == i_f_clampi_min_f);
            D_ASSERT(clampi_min_f == clampi_neg_inf_f);
            D_ASSERT(clampi_min_f == clampi_min_ff);

            D_ASSERT(clampf_min_i == f_i_clampf_min_i);
            D_ASSERT(clampf_min_i == clampf_ii_clampf_min_i);

            if (is_trunc)
            {
                D_ASSERT(clampi_min_f >= min_i);
                D_ASSERT(clampf_min_i >= min_f);
                D_ASSERT(i_clampf_min_i >= clampi_min_f);
                D_ASSERT(clampf_min_i >= f_clampi_min_f);
            }
            else
            {
                D_ASSERT(i_min_f == min_i);
                D_ASSERT(clampi_min_f == min_i);
                D_ASSERT(clampf_min_i == min_f);
                D_ASSERT(clampi_min_f == i_clampf_min_i);
                D_ASSERT(clampf_min_i == f_clampi_min_f);
            }
        }

        D_WARNING_POP;
    }
}

void test_clamp_cast() noexcept
{
    {
        static_assert(hi_cast<uint16_t>(0xdeadbeefUL) == 0xdeadU);
        static_assert(lo_cast<uint16_t>(0xdeadbeefUL) == 0xbeefU);
        static_assert(hi_cast<uint32_t>(0xfeedfacedeadbeefULL) == 0xfeedfaceUL);
        static_assert(lo_cast<uint32_t>(0xfeedfacedeadbeefULL) == 0xdeadbeefUL);

        static_assert(hi_cast<uint8_t>(0xdeadbeefUL) == 0xbeU);
        static_assert(lo_cast<uint8_t>(0xdeadbeefUL) == 0xefU);
        static_assert(hi_cast<uint32_t>(0xdeadbeefUL) == 0UL);
        static_assert(lo_cast<uint32_t>(0xdeadbeefUL) == 0xdeadbeefUL);
    }

    {
        static_assert(clamp_cast<uint64_t>(0xfeedfacedeadbeefULL) == 0xfeedfacedeadbeefULL);
        static_assert(clamp_cast<uint32_t>(0xfeedfacedeadbeefULL) == 0xffffffffUL);
        static_assert(clamp_cast<uint32_t>(0xdeadbeefULL) == 0xdeadbeefUL);
        static_assert(clamp_cast<uint16_t>(0xfeedfacedeadbeefULL) == 0xffffu);
        static_assert(clamp_cast<uint16_t>(0xbeefULL) == 0xbeefUL);
    }

    {
        constexpr auto u64_inf64 = clamp_cast<uint64_t>(numeric_inf_v<double>);
        constexpr auto u64_inf32 = clamp_cast<uint64_t>(numeric_inf_v<float>);
        constexpr auto u64_neg_inf64 = clamp_cast<uint64_t>(-numeric_inf_v<double>);
        constexpr auto u64_neg_inf32 = clamp_cast<uint64_t>(-numeric_inf_v<float>);
        D_ASSERT(!clamp_cast<uint64_t>(numeric_nan_v<double>));
        D_ASSERT(!clamp_cast<uint64_t>(numeric_nan_v<float>));

        constexpr auto u32_inf64 = clamp_cast<uint32_t>(numeric_inf_v<double>);
        constexpr auto u32_inf32 = clamp_cast<uint32_t>(numeric_inf_v<float>);
        constexpr auto u32_neg_inf64 = clamp_cast<uint32_t>(-numeric_inf_v<double>);
        constexpr auto u32_neg_inf32 = clamp_cast<uint32_t>(-numeric_inf_v<float>);
        D_ASSERT(!clamp_cast<uint32_t>(numeric_nan_v<double>));
        D_ASSERT(!clamp_cast<uint32_t>(numeric_nan_v<float>));

        constexpr auto u16_inf64 = clamp_cast<uint16_t>(numeric_inf_v<double>);
        constexpr auto u16_inf32 = clamp_cast<uint16_t>(numeric_inf_v<float>);
        constexpr auto u16_neg_inf64 = clamp_cast<uint16_t>(-numeric_inf_v<double>);
        constexpr auto u16_neg_inf32 = clamp_cast<uint16_t>(-numeric_inf_v<float>);
        D_ASSERT(!clamp_cast<uint16_t>(numeric_nan_v<double>));
        D_ASSERT(!clamp_cast<uint16_t>(numeric_nan_v<float>));

        constexpr auto i64_inf64 = clamp_cast<int64_t>(numeric_inf_v<double>);
        constexpr auto i64_inf32 = clamp_cast<int64_t>(numeric_inf_v<float>);
        const auto i64_neg_inf64 = clamp_cast<int64_t>(-numeric_inf_v<double>);
        const auto i64_neg_inf32 = clamp_cast<int64_t>(-numeric_inf_v<float>);
        D_ASSERT(!clamp_cast<int64_t>(numeric_nan_v<double>));
        D_ASSERT(!clamp_cast<int64_t>(numeric_nan_v<float>));

        constexpr auto i32_inf64 = clamp_cast<int32_t>(numeric_inf_v<double>);
        constexpr auto i32_inf32 = clamp_cast<int32_t>(numeric_inf_v<float>);
        constexpr auto i32_neg_inf64 = clamp_cast<int32_t>(-numeric_inf_v<double>);
        constexpr auto i32_neg_inf32 = clamp_cast<int32_t>(-numeric_inf_v<float>);
        D_ASSERT(!clamp_cast<int32_t>(numeric_nan_v<double>));
        D_ASSERT(!clamp_cast<int32_t>(numeric_nan_v<float>));

        constexpr auto i16_inf64 = clamp_cast<int16_t>(numeric_inf_v<double>);
        constexpr auto i16_inf32 = clamp_cast<int16_t>(numeric_inf_v<float>);
        constexpr auto i16_neg_inf64 = clamp_cast<int16_t>(-numeric_inf_v<double>);
        constexpr auto i16_neg_inf32 = clamp_cast<int16_t>(-numeric_inf_v<float>);
        D_ASSERT(!clamp_cast<int16_t>(numeric_nan_v<double>));
        D_ASSERT(!clamp_cast<int16_t>(numeric_nan_v<float>));

        static_assert(u64_inf64 == (0xffffffffffffffffULL & ~((1ULL << 11) - 1U)));
        static_assert(u64_inf32 == (0xffffffffffffffffULL & ~((1ULL << 40) - 1U)));
        static_assert(u64_neg_inf64 == 0ULL);
        static_assert(u64_neg_inf32 == 0ULL);

        static_assert(u32_inf64 == 0xffffffffUL);
        static_assert(u32_inf32 == (0xffffffffUL & ~((1UL << 8) - 1U)));
        static_assert(u32_neg_inf64 == 0ULL);
        static_assert(u32_neg_inf32 == 0ULL);

        static_assert(u16_inf64 == 0xffffu);
        static_assert(u16_inf32 == 0xffffu);
        static_assert(u16_neg_inf64 == 0ULL);
        static_assert(u16_neg_inf32 == 0ULL);

        static_assert(i64_inf64 == (0x7fffffffffffffffLL & ~((1LL << 10) - 1)));
        static_assert(i64_inf32 == (0x7fffffffffffffffLL & ~((1LL << 39) - 1)));
        static_assert(i64_neg_inf64 == -0x7fffffffffffffffLL - 1);
        static_assert(i64_neg_inf32 == -0x7fffffffffffffffLL - 1);

        static_assert(i32_inf64 == 0x7fffffff);
        static_assert(i32_inf32 == (0x7fffffff & ~((1 << 7) - 1)));
        static_assert(i32_neg_inf64 == -0x7fffffff - 1);
        static_assert(i32_neg_inf32 == -0x7fffffff - 1);

        static_assert(i16_inf64 == 0x7fff);
        static_assert(i16_inf32 == 0x7fff);
        static_assert(i16_neg_inf64 == -0x7fff - 1);
        static_assert(i16_neg_inf32 == -0x7fff - 1);
    }

    test_clamp_cast_for<uint8_t, uint8_t>();
    test_clamp_cast_for<uint8_t, uint16_t>();
    test_clamp_cast_for<uint8_t, uint32_t>();
    test_clamp_cast_for<uint8_t, uint64_t>();
    test_clamp_cast_for<uint8_t, int8_t>();
    test_clamp_cast_for<uint8_t, int16_t>();
    test_clamp_cast_for<uint8_t, int32_t>();
    test_clamp_cast_for<uint8_t, int64_t>();

    test_clamp_cast_for<uint16_t, uint8_t>();
    test_clamp_cast_for<uint16_t, uint16_t>();
    test_clamp_cast_for<uint16_t, uint32_t>();
    test_clamp_cast_for<uint16_t, uint64_t>();
    test_clamp_cast_for<uint16_t, int8_t>();
    test_clamp_cast_for<uint16_t, int16_t>();
    test_clamp_cast_for<uint16_t, int32_t>();
    test_clamp_cast_for<uint16_t, int64_t>();

    test_clamp_cast_for<uint32_t, uint8_t>();
    test_clamp_cast_for<uint32_t, uint16_t>();
    test_clamp_cast_for<uint32_t, uint32_t>();
    test_clamp_cast_for<uint32_t, uint64_t>();
    test_clamp_cast_for<uint32_t, int8_t>();
    test_clamp_cast_for<uint32_t, int16_t>();
    test_clamp_cast_for<uint32_t, int32_t>();
    test_clamp_cast_for<uint32_t, int64_t>();

    test_clamp_cast_for<uint64_t, uint8_t>();
    test_clamp_cast_for<uint64_t, uint16_t>();
    test_clamp_cast_for<uint64_t, uint32_t>();
    test_clamp_cast_for<uint64_t, uint64_t>();
    test_clamp_cast_for<uint64_t, int8_t>();
    test_clamp_cast_for<uint64_t, int16_t>();
    test_clamp_cast_for<uint64_t, int32_t>();
    test_clamp_cast_for<uint64_t, int64_t>();

    test_clamp_cast_for<int8_t, uint8_t>();
    test_clamp_cast_for<int8_t, uint16_t>();
    test_clamp_cast_for<int8_t, uint32_t>();
    test_clamp_cast_for<int8_t, uint64_t>();
    test_clamp_cast_for<int8_t, int8_t>();
    test_clamp_cast_for<int8_t, int16_t>();
    test_clamp_cast_for<int8_t, int32_t>();
    test_clamp_cast_for<int8_t, int64_t>();

    test_clamp_cast_for<int16_t, uint8_t>();
    test_clamp_cast_for<int16_t, uint16_t>();
    test_clamp_cast_for<int16_t, uint32_t>();
    test_clamp_cast_for<int16_t, uint64_t>();
    test_clamp_cast_for<int16_t, int8_t>();
    test_clamp_cast_for<int16_t, int16_t>();
    test_clamp_cast_for<int16_t, int32_t>();
    test_clamp_cast_for<int16_t, int64_t>();

    test_clamp_cast_for<int32_t, uint8_t>();
    test_clamp_cast_for<int32_t, uint16_t>();
    test_clamp_cast_for<int32_t, uint32_t>();
    test_clamp_cast_for<int32_t, uint64_t>();
    test_clamp_cast_for<int32_t, int8_t>();
    test_clamp_cast_for<int32_t, int16_t>();
    test_clamp_cast_for<int32_t, int32_t>();
    test_clamp_cast_for<int32_t, int64_t>();

    test_clamp_cast_for<int64_t, uint8_t>();
    test_clamp_cast_for<int64_t, uint16_t>();
    test_clamp_cast_for<int64_t, uint32_t>();
    test_clamp_cast_for<int64_t, uint64_t>();
    test_clamp_cast_for<int64_t, int8_t>();
    test_clamp_cast_for<int64_t, int16_t>();
    test_clamp_cast_for<int64_t, int32_t>();
    test_clamp_cast_for<int64_t, int64_t>();

    test_clamp_cast_for_f<int16_t, float>();
    test_clamp_cast_for_f<uint16_t, float>();
    test_clamp_cast_for_f<int32_t, float>();
    test_clamp_cast_for_f<uint32_t, float>();
    test_clamp_cast_for_f<int64_t, float>();
    test_clamp_cast_for_f<uint64_t, float>();

    test_clamp_cast_for_f<int16_t, double>();
    test_clamp_cast_for_f<uint16_t, double>();
    test_clamp_cast_for_f<int32_t, double>();
    test_clamp_cast_for_f<uint32_t, double>();
    test_clamp_cast_for_f<int64_t, double>();
    test_clamp_cast_for_f<uint64_t, double>();
}