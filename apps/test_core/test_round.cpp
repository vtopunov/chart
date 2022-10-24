#include <core/round.h>

#include <core/assert.h>

#include <string>


void test_round() noexcept
{
    static_assert(24 == numeric_digits_v<float>);
    D_ASSERT(16777215L == round_cast<int32_t>(16777215.0f));
    D_ASSERT(16777215L == round_cast<int32_t>(16777215.4f));
    D_ASSERT(16777216L == round_cast<int32_t>(16777215.5f));
    D_ASSERT(16777216L == round_cast<int32_t>(16777216.1f));
    D_ASSERT(16777216L == round_cast<int32_t>(16777216.5f));
    D_ASSERT(16777216L == round_cast<int32_t>(16777217.0f));

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