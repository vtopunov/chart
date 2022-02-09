
#include<core/utf.h>

using namespace std::string_view_literals;

void test_utf() noexcept
{
    auto u8s = u8"Hello 🌍! Привет 🌍! 你好世界!"sv;
    auto u32s = U"Hello 🌍! Привет 🌍! 你好世界!"sv;

    decode_utf<sizeof(char32_t)>(u8s, [&u32s](char32_t ch)
        {
            D_ASSERT(u32s.front() == ch);
            u32s.remove_prefix(1u);
        }
    );

    D_ASSERT(!u32s.size());
}