#include <core/fwd.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) noexcept
{
    double x = 0.1 + 0.2;
    D_ASSUME(x < x * x);
    return static_cast<int>(x);
}