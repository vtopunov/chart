#include <core/math.h>


namespace
{
    struct test_eq_op
    {
        [[nodiscard]] constexpr bool operator == (const test_eq_op&) const noexcept
        {
            return true;
        }
        [[nodiscard]] constexpr bool operator != (const test_eq_op&) const noexcept = delete;
    };

    struct test_neq_op
    {
        [[nodiscard]] constexpr bool operator == (const test_eq_op&) const noexcept = delete;
        [[nodiscard]] constexpr bool operator != (const test_neq_op&) const noexcept
        {
            return true;
        }
    };
}

void test_math() noexcept
{
    const struct __errno_holder__
    {
        const int value{ errno };

        ~__errno_holder__() noexcept
        {
            errno = value;
        }
    } hold_errno;

    constexpr auto real_eps = numeric_eps_v<double>;

    {
        constexpr double test_real_nearz_values[]
        {
            -(3.0 * real_eps)
            - (2.0 * real_eps)
            - real_eps,
            0.0,
            real_eps,
            2.0 * real_eps,
            3.0 * real_eps
        };

        for(const auto nearz : test_real_nearz_values )
        {
            const auto prev = prevfp(nearz);
            const auto prev_prev = prevfp(prev);
            const auto next = nextfp(nearz);
            const auto next_next = nextfp(next);

            {
                D_ASSERT(!is_less_neqfp(nearz, nearz));

                D_ASSERT(!is_less_neqfp(prev, nearz));
                D_ASSERT(is_less_neqfp(prev_prev, nearz));

                D_ASSERT(!is_less_neqfp(nearz, next));
                D_ASSERT(is_less_neqfp(nearz, next_next));

                D_ASSERT(is_less_neqfp(prev, next));
            }

            {
                D_ASSERT(!is_greater_neqfp(nearz, nearz));

                D_ASSERT(!is_greater_neqfp(next, nearz));
                D_ASSERT(is_greater_neqfp(next_next, nearz));

                D_ASSERT(!is_greater_neqfp(nearz, prev));
                D_ASSERT(is_greater_neqfp(nearz, prev_prev));

                D_ASSERT(is_greater_neqfp(next, prev));
            }

            {
                D_ASSERT(!is_neqfp(nearz, nearz));
                D_ASSERT(!is_neqfp(prev, nearz));
                D_ASSERT(!is_neqfp(nearz, prev));
                D_ASSERT(!is_neqfp(next, nearz));
                D_ASSERT(!is_neqfp(nearz, next));
                D_ASSERT(is_neqfp(prev_prev, nearz));
                D_ASSERT(is_neqfp(nearz, prev_prev));
                D_ASSERT(is_neqfp(next_next, nearz));
                D_ASSERT(is_neqfp(nearz, next_next));
                D_ASSERT(is_neqfp(prev, next));
                D_ASSERT(is_neqfp(next, prev));
            }

            {
                D_ASSERT(is_eqfp(nearz, nearz));
                D_ASSERT(is_eqfp(prev, nearz));
                D_ASSERT(is_eqfp(nearz, prev));
                D_ASSERT(is_eqfp(next, nearz));
                D_ASSERT(is_eqfp(nearz, next));
                D_ASSERT(!is_eqfp(prev_prev, nearz));
                D_ASSERT(!is_eqfp(nearz, prev_prev));
                D_ASSERT(!is_eqfp(next_next, nearz));
                D_ASSERT(!is_eqfp(nearz, next_next));
                D_ASSERT(!is_eqfp(prev, next));
                D_ASSERT(!is_eqfp(next, prev));
            }

            {
                D_ASSERT(!is_neqn(nearz, nearz));
                D_ASSERT(!is_neqn(prev, nearz));
                D_ASSERT(!is_neqn(nearz, prev));
                D_ASSERT(!is_neqn(next, nearz));
                D_ASSERT(!is_neqn(nearz, next));
                D_ASSERT(is_neqn(prev_prev, nearz));
                D_ASSERT(is_neqn(nearz, prev_prev));
                D_ASSERT(is_neqn(next_next, nearz));
                D_ASSERT(is_neqn(nearz, next_next));
                D_ASSERT(is_neqn(prev, next));
                D_ASSERT(is_neqn(next, prev));
            }
        }
    }

    {
        D_ASSERT(is_neqn(test_neq_op{}, test_neq_op{}));
        D_ASSERT(!is_neqn(test_eq_op{}, test_eq_op{}));
    }
}