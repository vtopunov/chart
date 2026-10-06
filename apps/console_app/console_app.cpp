#include <print>

#include <core/buffer.h>

#include <mimalloc.h>


template<size_t Align>
void test_gs() noexcept
{
    constexpr size_t a = Align;
    const auto s = a + 1;
    const auto gs = mi_good_size(s);
    const auto as = size_align<a>(s);
    const auto gas = mi_good_size(as);
    const auto m = mi_malloc(s);
    const auto am = mi_malloc_aligned(s, a);
    const auto us = mi_usable_size(m);
    const auto uas = mi_usable_size(am);

    using abuffer = basic_buffer<std::byte, a>;
    abuffer buf{ s };
    const auto gasb = abuffer::good_size(s);
    const auto uasb = buf.size();
    const auto uasbb = buf.size_bytes();

    std::println("a = {}, s = {}, gs = {}, as = {}, gas = {}, us = {}, uas = {}, gasb = {}, uasb = {}, uasbb = {}",
        a, s, gs, as, gas, us, uas, gasb, uasb, uasbb);

    D_ASSERT(gas == uas);
    D_ASSERT(gas == gasb);
    D_ASSERT(gas == uasb);
    D_ASSERT(gas == uasbb);

    mi_free(m);
    mi_free(am);
}


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) noexcept
{
    double x = 0.1 + 0.2;
    D_ASSUME(x < x * x);

    test_gs<512u>();
    test_gs<256u>();
    test_gs<128u>();
    test_gs<64u>();
    test_gs<32u>();
    test_gs<16u>();
    test_gs<8u>();
    test_gs<4u>();
    
    return 0;
}