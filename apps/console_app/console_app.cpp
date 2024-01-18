#include <type_traits>

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
    static_assert(std::is_convertible_v<void*, int*>);

    int value{ 2 };
    S_ini v{ .value_ref{ value } };
    v.value_ref = !nullptr;
    constexpr std::nullptr_t np{};
    return test<int*>(np); 
}