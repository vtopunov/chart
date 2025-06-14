#include <execution>
#include <algorithm>
#include <bit>


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) noexcept
{
    size_t args_len{};

    const auto slrlen_acc = [&args_len] (const char* arg) noexcept 
    {
          args_len += strlen(arg);
    };

    std::for_each
    (
        std::execution::unseq,
        argv, argv + argc,
        slrlen_acc
    );

    return std::popcount(args_len);
}