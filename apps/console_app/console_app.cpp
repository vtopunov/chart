#include <type_traits>

#include <cmath>
#include <limits>

struct O_ini
{
    constexpr O_ini() noexcept
    {}

    constexpr ~O_ini() noexcept
    {}
};

struct S_ini : O_ini 
{
    int& value_ref;

    constexpr ~S_ini() noexcept
    {
        value_ref = 0;
    }
};

template<class T>
int test(const T& pn) noexcept
{
    return *pn + 3;
}

int main() noexcept
{
    auto isn = std::isnormal(std::nexttoward(std::numeric_limits<double>::epsilon(), std::numeric_limits<long double>::lowest()));

    int value{ 2 + isn };
    S_ini v{ .value_ref{ value } };
    v.value_ref = !nullptr;
    constexpr std::nullptr_t np{};
    return test<int*>(np); 
}