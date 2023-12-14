#include <core/utility.h>
#include <core/assert.h>

#include <cerrno>

namespace
{
    namespace private_detail_test_swap
    {
        constexpr size_t swap_S0_S0{ 1u << 0 };
        constexpr size_t swap_S1_S0{ 1u << 1 };
        constexpr size_t swap_S1_S1{ 1u << 2 };


        struct S0
        {
            size_t& ctx;

            void swap(S0&) noexcept
            {
                ctx |= swap_S0_S0;
            }
        };

        struct S1 : S0
        {
            void swap(S0&) noexcept
            {
                ctx |= swap_S1_S0;
            }

            void swap(S1&) noexcept
            {
                ctx |= swap_S1_S1;
            }
        };
    }

    void test_swap() noexcept
    {
        using namespace private_detail_test_swap;

        size_t ctx{ 0u };
        S0 s0{ ctx };
        S1 s1{ ctx };

        {
            ctx = {};
            swap(s0, s0);
            D_ASSERT(swap_S0_S0 == ctx);
        }

        {
            ctx = {};
            swap(s1, s0);
            D_ASSERT(swap_S1_S0 == ctx);
        }

        {
            ctx = {};
            swap(s0, s1);
            D_ASSERT(swap_S1_S0 == ctx);
        }

        {
            ctx = {};
            swap(s1, s1);
            D_ASSERT(swap_S1_S1 == ctx);
        }
    }
}

void test_utility() noexcept
{
    test_swap();
}