#include <iostream>
#include <iomanip>
#include <bit>
#include <cmath>
#include <numbers>
#include <span>
#include <optional>
#include <fstream>
#include <sstream>
#include <charconv>
#include <vector>
#include <map>
#include <variant>
#include <array>
#include <cuchar>

// #include <core/intrusive_list.h>
// #include <core/small_vector.h>
// #include <core/point2d.h>

struct ps
{
    int x{ 1 };

private:
    int y{ 0 };
};

int main() noexcept
{
    using namespace std::string_literals;

    std::wstring s;
    constexpr auto sz = sizeof(s);

    char xz[500] = { 1 };

    double d{ 0.0 };
    --d;

    using signed_t = std::common_type_t<signed, unsigned>;

    constexpr std::make_signed_t<unsigned> i{ 0 };


    return xz[0];
}
