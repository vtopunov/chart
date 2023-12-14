#include <core/round.h>

#include <core/assert.h>

#include <string>

namespace
{
    void test_round_cast() noexcept
    {
        static_assert(24 == numeric_digits_v<float>);
        D_ASSERT(16777215L == round_cast<int32_t>(16777215.0f));
        D_ASSERT(16777215L == round_cast<int32_t>(16777215.4f));
        D_ASSERT(16777216L == round_cast<int32_t>(16777215.5f));
        D_ASSERT(16777216L == round_cast<int32_t>(16777216.1f));
        D_ASSERT(16777216L == round_cast<int32_t>(16777216.5f));
        D_ASSERT(16777216L == round_cast<int32_t>(16777217.0f));
        D_ASSERT(16777218L == round_cast<int32_t>(16777218.0f));

        static_assert(15 == numeric_digits_v<int16_t>);
        D_ASSERT(32767 == round_cast<int16_t>(32767.0f));
        D_ASSERT(32767 == round_cast<int16_t>(32768.0f));

        static_assert(16 == numeric_digits_v<uint16_t>);
        D_ASSERT(65535u == round_cast<uint16_t>(65535.0f));
        D_ASSERT(65535u == round_cast<uint16_t>(65536.0f));

        static_assert(53 == numeric_digits_v<double>);
        D_ASSERT(9007199254740991LL == round_cast<int64_t>(9007199254740991.0));
        D_ASSERT(9007199254740991LL == round_cast<int64_t>(9007199254740991.4));
        D_ASSERT(9007199254740992LL == round_cast<int64_t>(9007199254740991.5));
        D_ASSERT(9007199254740992LL == round_cast<int64_t>(9007199254740992.1));
        D_ASSERT(9007199254740992LL == round_cast<int64_t>(9007199254740992.5));
        D_ASSERT(9007199254740992LL == round_cast<int64_t>(9007199254740993.0));

        D_ASSERT(0xffffff00ul == round_cast<uint32_t>(4.3123e9f));

        {
            const auto s_round = std::to_string(round_cast<uint64_t>(4.3123e9f));
            D_ASSERT(10u == s_round.size());
            D_ASSERT(s_round.starts_with("431230"));
        }


        D_ASSERT(0UL == round_cast<uint32_t>(-1.0f));
        D_ASSERT(0ULL == round_cast<uint64_t>(-1.0f));
        D_ASSERT(0UL == round_cast<uint32_t>(-1.0));
        D_ASSERT(0ULL == round_cast<uint64_t>(-1.0));
    }

    void test_round_to_near() noexcept
    {
        {
            static_assert(std::is_same_v<int, decltype(round_to_near(0.1, 1))>);
            static_assert(std::is_same_v<int, decltype(round_to_near(0.1f, 1))>);
            D_ASSERT(-2 == round_to_near(-2.9, -1));
            D_ASSERT(-2 == round_to_near(-2.1, -1));
            D_ASSERT(-1 == round_to_near(-1.9, -1));
            D_ASSERT(-1 == round_to_near(-1.1, -1));
            D_ASSERT(-1 == round_to_near(-0.9, -1));
            D_ASSERT(-1 == round_to_near(-0.1, -1));
            D_ASSERT(0 == round_to_near(0.1, -1));
            D_ASSERT(0 == round_to_near(0.9, -1));
            D_ASSERT(1 == round_to_near(1.1, -1));
            D_ASSERT(1 == round_to_near(1.9, -1));

            D_ASSERT(-1 == round_to_near(-1.9, 0));
            D_ASSERT(-1 == round_to_near(-1.1, 0));
            D_ASSERT(0 == round_to_near(-0.9, 0));
            D_ASSERT(0 == round_to_near(-0.1, 0));
            D_ASSERT(0 == round_to_near(0.1, 0));
            D_ASSERT(0 == round_to_near(0.9, 0));
            D_ASSERT(1 == round_to_near(1.1, 0));
            D_ASSERT(1 == round_to_near(1.9, 0));

            D_ASSERT(0u == round_to_near(-1.9, 0u));
            D_ASSERT(0u == round_to_near(-1.1, 0u));
            D_ASSERT(0u == round_to_near(-0.9, 0u));
            D_ASSERT(0u == round_to_near(-0.1, 0u));
            D_ASSERT(0u == round_to_near(0.1, 0u));
            D_ASSERT(0u == round_to_near(0.9, 0u));
            D_ASSERT(1u == round_to_near(1.1, 0u));
            D_ASSERT(1u == round_to_near(1.9, 0u));

            D_ASSERT(0 == round_to_near(-0.9, 1));
            D_ASSERT(0 == round_to_near(-0.1, 1));
            D_ASSERT(1 == round_to_near(0.1, 1));
            D_ASSERT(1 == round_to_near(0.9, 1));
            D_ASSERT(1 == round_to_near(1.1, 1));
            D_ASSERT(1 == round_to_near(1.9, 1));
            D_ASSERT(2 == round_to_near(2.1, 1));
            D_ASSERT(2 == round_to_near(2.9, 1));

            D_ASSERT(1 == round_to_near(0.1, 2));
            D_ASSERT(1 == round_to_near(0.9, 2));
            D_ASSERT(2 == round_to_near(1.1, 2));
            D_ASSERT(2 == round_to_near(1.9, 2));
            D_ASSERT(2 == round_to_near(2.1, 2));
            D_ASSERT(2 == round_to_near(2.9, 2));
            D_ASSERT(3 == round_to_near(3.1, 2));
            D_ASSERT(3 == round_to_near(3.9, 2));
        }

        {
            D_ASSERT(0x7fffffff == round_to_near(numeric_inf_v<double>, 0));
            D_ASSERT(0x7fffffff == round_to_near(numeric_inf_v<double>, 0x7fffffff));
            D_ASSERT(0x7fffffff == round_to_near(numeric_inf_v<double>, 0x7ffffffe));

            {
                constexpr auto i_min = -0x7fffffff - 1;
                D_ASSERT((i_min + 1) == round_to_near(-numeric_inf_v<double>, 0));
                D_ASSERT(i_min == round_to_near(-numeric_inf_v<double>, i_min));
                D_ASSERT((i_min + 1) == round_to_near(-numeric_inf_v<double>, i_min + 1));
            }

            D_ASSERT(0u == round_to_near(-numeric_inf_v<double>, 0u));
            D_ASSERT(1u == round_to_near(-numeric_inf_v<double>, 1u));

            D_ASSERT(0 == round_to_near(numeric_nan_v<double>, -1));
            D_ASSERT(0 == round_to_near(numeric_nan_v<double>, 0));
            D_ASSERT(1 == round_to_near(numeric_nan_v<double>, 1));

            D_ASSERT(0u == round_to_near(numeric_nan_v<double>, 0u));
            D_ASSERT(1u == round_to_near(numeric_nan_v<double>, 1u));
        }

        {
            static_assert(15 == numeric_digits_v<int16_t>);
            D_ASSERT(32767 == round_to_near(32767.0f, int16_t(32767)));
            D_ASSERT(32767 == round_to_near(32768.0f, int16_t(32767)));
            D_ASSERT(32767 == round_to_near(32769.0f, int16_t(32767)));
            D_ASSERT(-32768 == round_to_near(-32768.0f, int16_t(-32768)));
            D_ASSERT(-32768 == round_to_near(-32769.0f, int16_t(-32768)));
            D_ASSERT(-32768 == round_to_near(-32770.0f, int16_t(-32768)));
        }

        {
            static_assert(-3.9 == round_to_near(-3.9, 4.0));
            static_assert(3.9 == round_to_near(3.9, 4.0));
            static_assert(-2 == round_to_near(-2, 2));
            static_assert(2 == round_to_near(2, 2));
            static_assert(2.0 == round_to_near(2, 2.1));
            static_assert(0u == round_to_near(-1, 2u));
        }
    }
}

void test_round() noexcept
{
    test_round_cast();
    test_round_to_near();
}