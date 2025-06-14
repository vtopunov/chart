#include <chart/space_diagonal_cache.h>


void test_space_diagonal_cache() noexcept
{
    {
        using chart::private_detail_space_diagonal_cache::inrange_neqfp;

        D_ASSERT(!inrange_neqfp(0.0, 0.0, 0.0));
        D_ASSERT(!inrange_neqfp(0.0, prevfp(0.0), 0.0));
        D_ASSERT(!inrange_neqfp(0.0, 0.0, nextfp(0.0)));
        D_ASSERT(!inrange_neqfp(0.0, prevfp(0.0), nextfp(0.0)));
        D_ASSERT(!inrange_neqfp(0.0, prevfp(prevfp(0.0)), nextfp(0.0)));
        D_ASSERT(!inrange_neqfp(0.0, prevfp(0.0), nextfp(nextfp(0.0))));
        D_ASSERT(inrange_neqfp(0.0, prevfp(prevfp(0.0)), nextfp(nextfp(0.0))));
    }
}