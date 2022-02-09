#include <core/size_type.h>

namespace
{
    template<size_t aling>
    void test_size_aling() noexcept
    {
        constexpr auto stop = 0x10000_uz;

        static_assert(size_align<aling>(0_uz) == 0_uz);

        for (size_t aligned = 0; aligned <= stop; )
        {
            const auto next_aligned = aligned + aling;

            for (size_t unaligned = aligned + 1_uz; unaligned <= next_aligned; ++unaligned)
            {
                D_ASSERT(size_align<aling>(unaligned) == next_aligned);
            }

            aligned = next_aligned;
        }
    }
}

void test_size_type() noexcept
{
    test_size_aling<1_uz>();
    test_size_aling<2_uz>();
    test_size_aling<4_uz>();
    test_size_aling<8_uz>();
    test_size_aling<16_uz>();
}
