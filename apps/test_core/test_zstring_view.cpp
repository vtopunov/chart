#include "core/zstring_view.h"
#include "core/assert.h"

#include <string_view>
#include <string>

namespace
{
    template<bool test_value, class T>
    constexpr bool test_has_c_str() noexcept
    {
        static_assert(test_value == has_c_str<T>::value);
        static_assert(test_value == has_c_str<const T>::value);
        static_assert(test_value == has_c_str<T&>::value);
        static_assert(test_value == has_c_str<const T&>::value);
        static_assert(test_value == has_c_str<T&&>::value);
        static_assert(test_value == has_c_str<const T&&>::value);
        return true;
    }

    template<bool test_value, class T>
    constexpr bool test_is_zstring_view_compatible() noexcept
    {
        static_assert(test_value == is_zstring_view_compatible_v<T>);
        static_assert(test_value == is_zstring_view_compatible_v<const T>);
        static_assert(test_value == is_zstring_view_compatible_v<T&>);
        static_assert(test_value == is_zstring_view_compatible_v<const T&>);
        static_assert(test_value == is_zstring_view_compatible_v<T&&>);
        static_assert(test_value == is_zstring_view_compatible_v<const T&&>);
        return true;
    }
}

void test_zstring_view() noexcept
{
    struct my_string
    {
        constexpr const char* c_str() const noexcept
        {
            return "my_string";
        }
    };

    static_assert(test_has_c_str<true, my_string>());
    static_assert(test_has_c_str<true, zstring_view>());
    static_assert(test_has_c_str<true, std::string>());
    static_assert(test_has_c_str<false, std::string_view>());

    static_assert(!is_zstring_view<my_string>::value);
    static_assert(is_zstring_view<zstring_view>::value);
    static_assert(!is_zstring_view<std::string>::value);
    static_assert(!is_zstring_view<std::string_view>::value);

    static_assert(test_is_zstring_view_compatible<true, my_string>());
    static_assert(test_is_zstring_view_compatible<false, zstring_view>());
    static_assert(test_is_zstring_view_compatible<true, std::string>());
    static_assert(test_is_zstring_view_compatible<false, std::string_view>());

    {
        constexpr zstring_view empty_zsv;
        static_assert(!*(empty_zsv.c_str()));
    }

    {
        std::string s0{ "test0" }, s1{ "test1" };
        zstring_view s_zsv{ s0 };
        D_ASSERT(s_zsv.c_str() == s0.c_str());
        s_zsv = s1;
        D_ASSERT(s_zsv.c_str() == s1.c_str());
    }

    {
        const char* cs0 = "xz test0";
        const char* cs1 = "xz test1";
        zstring_view cs_zsv{ cs0 };
        D_ASSERT(cs_zsv.c_str() == cs0);
        cs_zsv = cs1;
        D_ASSERT(cs_zsv.c_str() == cs1);
    }

    D_ASSERT(!strcmp(("xz string literal"_zsv).c_str(), "xz string literal"));
}