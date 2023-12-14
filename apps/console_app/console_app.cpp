#include <type_traits>

struct snp
{
    std::nullptr_t np0;
    std::nullptr_t np1;
};

int main() noexcept
{
    int a = 0, b = 1;
    using type_meq = decltype(a -= b);

    snp snp0;

    return 0;
}