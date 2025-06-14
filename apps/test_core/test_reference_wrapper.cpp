#include <core/reference_wrapper.h>


void test_reference_wrapper() noexcept
{
    int value = 0;
    optional_reference_wrapper<int> nullorefwv{};
    optional_reference_wrapper<int> orefwv(value);
    optional_reference_wrapper<const int> orefwvc(value);
    D_ASSERT(!nullorefwv);
    D_ASSERT(orefwv);
    D_ASSERT(orefwvc);
    static_assert(std::is_same_v<int&, decltype(unorefwrap(orefwv))>);
    static_assert(std::is_same_v<const int&, decltype(unorefwrap(orefwvc))>);
    D_ASSERT(!errno);
}

