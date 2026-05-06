
#include <execution>
#include <algorithm>
#include <bit>
#include <initializer_list>
#include <memory>
#include <optional>
#include <functional>
#include <map>
#include <print>

#include <core/unique_function.h>

namespace
{
    struct xzs
    {
        D_DEFAULT_ALL_CA(xzs);
    };

    struct xzs2
    {
        xzs xz;
        D_DEFAULT_ALL_CAEQ(xzs2);
    };
}


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) noexcept
{
    xzs2 xz;
    enum xzxx
    {
        lol
    };

    std::is_integral_v<xzxx>;

    double x = 0.1 + 0.2;
    char buf[100];
    std::to_chars(buf, buf + 100, x);

    return 0;
}