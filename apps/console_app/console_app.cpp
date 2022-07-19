
#include <utility>

namespace
{
    struct S_with
    {
        bool test() const noexcept
        {
            return true;
        }
    };

    struct S_without {};

    template<class T>
    auto test_if_exist(T& tested) noexcept -> decltype(std::declval<T&>().test())
    {
        return tested.test();
    }

    struct no_overload
    {
        template<class T>
        constexpr no_overload(const T&) noexcept
        {}
    };

    bool test_if_exist(no_overload) noexcept
    {
        return false;
    }
}

int main() noexcept
{
    S_with sw{};
    S_without swo{};
    //no_overload osw = sw;

    const auto r0 = test_if_exist(sw);
    const auto r1 = test_if_exist(swo);

    return 0;
}
