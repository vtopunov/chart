#include <charconv>

int main() noexcept
{
    constexpr uint16_t xz{ 0x123 };
    char bytes[5];
    std::to_chars(std::begin(bytes), std::end(bytes), xz, 16);
    return 0;
}
