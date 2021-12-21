#include <core/utility.h>
#include <core/assert.h>

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


void test_utility() noexcept
{
    {
        static_assert(constexpr_abs(0) == 0);
        static_assert(constexpr_abs(0u) == 0u);
        static_assert(constexpr_abs(0_uz) == 0_uz);
        static_assert(constexpr_abs(0.0f) == 0.0f);
        
        static_assert(std::bit_cast<uint32_t>(0.0f) != std::bit_cast<uint32_t>(-0.0f));
        static_assert(std::bit_cast<uint32_t>(constexpr_abs(-0.0f)) == std::bit_cast<uint32_t>(-0.0f));
        static_assert(std::bit_cast<uint32_t>(constexpr_abs(0.0f)) == std::bit_cast<uint32_t>(0.0f));
        
        static_assert(std::bit_cast<uint64_t>(0.0) != std::bit_cast<uint64_t>(-0.0));
        static_assert(std::bit_cast<uint64_t>(constexpr_abs(-0.0)) == std::bit_cast<uint64_t>(-0.0));
        static_assert(std::bit_cast<uint64_t>(constexpr_abs(0.0)) == std::bit_cast<uint64_t>(0.0));

        static_assert(constexpr_abs(numeric_max_v<uint32_t>) == numeric_max_v<uint32_t>);
        static_assert(constexpr_abs(numeric_max_v<int32_t>) == numeric_max_v<int32_t>);
        static_assert(constexpr_abs(-numeric_max_v<int32_t>) == numeric_max_v<int32_t>);
    }

    {
        static_assert(std::is_same_v<copy_const_t<const int, int>, const int>);
        static_assert(std::is_same_v<copy_const_t<int, int>, int>);
        static_assert(std::is_same_v<copy_const_t<const int, char>, const char>);
        static_assert(std::is_same_v<copy_const_t<int, char>, char>);
        static_assert(std::is_same_v<copy_const_t<const int, unsigned>, const unsigned>);
        static_assert(std::is_same_v<copy_const_t<int, unsigned>, unsigned>);

        static_assert(std::is_same_v<copy_pointer_t<int*, int>, int*>);
        static_assert(std::is_same_v<copy_pointer_t<int, int>, int>);
        static_assert(std::is_same_v<copy_pointer_t<int*, char>, char*>);
        static_assert(std::is_same_v<copy_pointer_t<int, char>, char>);
        static_assert(std::is_same_v<copy_pointer_t<int*, unsigned>, unsigned*>);
        static_assert(std::is_same_v<copy_pointer_t<int, unsigned>, unsigned>);

        static_assert(std::is_same_v<replace_t<int, unsigned, char>, int>);
        static_assert(std::is_same_v<replace_t<unsigned, unsigned, char>, char>);
        static_assert(std::is_same_v<replace_t<char, unsigned, char>, char>);

        static_assert(std::is_same_v<make_unsigned_opt_t<float>, float>);
        static_assert(std::is_same_v<make_unsigned_opt_t<int>, unsigned>);
        static_assert(std::is_same_v<make_unsigned_opt_t<unsigned>, unsigned>);
    }

    {
        static_assert(hi_cast<uint16_t>(0xdeadbeeful) == 0xdeadu);
        static_assert(lo_cast<uint16_t>(0xdeadbeeful) == 0xbeefu);
        static_assert(hi_cast<uint32_t>(0xfeedfacedeadbeefull) == 0xfeedfaceul);
        static_assert(lo_cast<uint32_t>(0xfeedfacedeadbeefull) == 0xdeadbeeful);

        static_assert(hi_cast<uint8_t>(0xdeadbeeful) == 0xdeu);
        static_assert(lo_cast<uint8_t>(0xdeadbeeful) == 0xefu);
        static_assert(hi_cast<uint32_t>(0xdeadbeeful) == 0ul);
        static_assert(lo_cast<uint32_t>(0xdeadbeeful) == 0xdeadbeeful);
    }

    {
        static_assert(clamp_cast<uint64_t>(0xfeedfacedeadbeefull) == 0xfeedfacedeadbeefull);
        static_assert(clamp_cast<uint32_t>(0xfeedfacedeadbeefull) == 0xfffffffful);
        static_assert(clamp_cast<uint32_t>(0xdeadbeefull) == 0xdeadbeeful);
        static_assert(clamp_cast<uint16_t>(0xfeedfacedeadbeefull) == 0xffffu);
        static_assert(clamp_cast<uint16_t>(0xbeefull) == 0xbeeful);
    }

    {
        test_size_aling<1_uz>();
        test_size_aling<2_uz>();
        test_size_aling<4_uz>();
        test_size_aling<8_uz>();
        test_size_aling<16_uz>();
    }

    {
        struct s0_t { int v; };
        struct s1_s0_t : s0_t {};
        s0_t s0{ 3 };
        s1_s0_t s1_s0{ 5 };
        
        std::is_same_v<decltype(swap(s0, s1_s0)), s0_t&>;
        std::is_same_v<decltype(swap(s1_s0, s0)), s0_t&>;
        
        D_ASSERT(swap(s0, s1_s0).v == 5);
        D_ASSERT(s0.v == 5);
        D_ASSERT(s1_s0.v == 3);

        D_ASSERT(swap(s1_s0, s0).v == 3);
        D_ASSERT(s0.v == 3);
        D_ASSERT(s1_s0.v == 5);

        static size_t swap_counter{ 0_uz };

        struct swp_s0_t
        {
            int v;

            void swap(s0_t& s0)
            {
                std::swap(v, s0.v);
                ++swap_counter;
            }
        };

        swp_s0_t swp_s0{ 5 };
        std::is_same_v<decltype(swap(s0, swp_s0)), swp_s0_t&>;
        std::is_same_v<decltype(swap(swp_s0, s0)), swp_s0_t&>;

        D_ASSERT(swap(s0, swp_s0).v == 3);
        D_ASSERT(s0.v == 5);
        D_ASSERT(swp_s0.v == 3);
        D_ASSERT(swap_counter == 1_uz);
        
        D_ASSERT(swap(swp_s0, s0).v == 5);
        D_ASSERT(s0.v == 3);
        D_ASSERT(swp_s0.v == 5);
        D_ASSERT(swap_counter == 2_uz);
    }

    D_ASSERT(!errno);
}