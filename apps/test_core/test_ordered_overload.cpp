#include <core/ordered_overload.h>
#include <core/assert.h>

#include <utility>
#include <cerrno>

namespace
{
    struct with_method
    {
        constexpr void method() const noexcept
        {}
    };

    struct without_method {};

    template<class T>
    constexpr auto call_method_if_exist(T& tested) noexcept -> decltype((std::declval<T&>().method(), true))
    {
        tested.method();
        return true;
    }

    constexpr bool call_method_if_exist(no_overloaded) noexcept
    {
        return false;
    }
}

void test_ordered_overload() noexcept
{
    constexpr with_method swt{};
    constexpr without_method swot{};

    static_assert(call_method_if_exist(swt));
    static_assert(!call_method_if_exist(swot));


    D_ASSERT(!errno);
}