#include <core/temp_swap.h>

#include <core/assert.h>
#include <core/size_type.h>

namespace
{
    namespace testing
    {
        using private_detail_swap::swap;
    }
}

void test_temp_swap() noexcept
{
    struct s0_t { int v; };
    struct s1_s0_t : s0_t {};
    s0_t s0{ 3 };
    s1_s0_t s1_s0{ 5 };

    std::is_same_v<decltype(testing::swap(s0, s1_s0)), s0_t&>;
    std::is_same_v<decltype(testing::swap(s1_s0, s0)), s0_t&>;

    D_ASSERT(testing::swap(s0, s1_s0).v == 5);
    D_ASSERT(s0.v == 5);
    D_ASSERT(s1_s0.v == 3);

    D_ASSERT(testing::swap(s1_s0, s0).v == 3);
    D_ASSERT(s0.v == 3);
    D_ASSERT(s1_s0.v == 5);

    static size_t swap_counter{0_uz};

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
    std::is_same_v<decltype(testing::swap(s0, swp_s0)), swp_s0_t&>;
    std::is_same_v<decltype(testing::swap(swp_s0, s0)), swp_s0_t&>;

    D_ASSERT(testing::swap(s0, swp_s0).v == 3);
    D_ASSERT(s0.v == 5);
    D_ASSERT(swp_s0.v == 3);
    D_ASSERT(swap_counter == 1_uz);

    D_ASSERT(testing::swap(swp_s0, s0).v == 5);
    D_ASSERT(s0.v == 3);
    D_ASSERT(swp_s0.v == 5);
    D_ASSERT(swap_counter == 2_uz);

    D_ASSERT(!errno);
}