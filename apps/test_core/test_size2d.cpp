
#include<core/size2d.h>

void test_size2d() noexcept
{
    constexpr vec2 v{ 3, 4 };
    constexpr size2d sz{ v };
    static_assert(std::is_same_v<decltype(sz), const size2d<int>>);
    static_assert(std::is_trivial_v<size2d<int>> && std::is_standard_layout_v<size2d<int>>);
    static_assert(sz.width() == 3);
    static_assert(sz.height() == 4);

    constexpr vec2 uv{ 3u, 4u };
    constexpr size2d usz{ uv };
    static_assert(std::is_same_v<decltype(usz), const size2d<unsigned>>);

    static_assert(md_narrow<size2d<int>>(usz) == sz);
    static_assert(md_narrow<vec2<int>>(usz) == v);
    static_assert(md_narrow<size2d<unsigned>>(sz) == usz);
    static_assert(md_narrow<vec2<unsigned>>(sz) == uv);

    constexpr size2d csz{ '\5', '\6' };
    static_assert(std::is_same_v<decltype(csz), const size2d<char>>);
    static_assert(csz.width() == '\5');
    static_assert(csz.height() == '\6');

    D_ASSERT(!errno);
}