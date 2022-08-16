#include <core/static_vector.h>


void test_static_vector() noexcept
{
    constexpr auto size = 4_uz;
    static_vector<int, size> sv;
    D_ASSERT(0 == sv.size());
    D_ASSERT(size == sv.capacity());
    D_ASSERT(sv.try_emplace_back(0));
    D_ASSERT(sv.try_emplace_back(1));
    D_ASSERT(sv.try_emplace_back(2));
    D_ASSERT(sv.try_emplace_back(3));
    D_ASSERT(size == sv.size());
    D_ASSERT(size == sv.capacity());
    D_ASSERT(!sv.try_emplace_back(4));
    D_ASSERT(!sv.try_emplace_back(5));
    D_ASSERT(!sv.try_emplace_back(6));
    D_ASSERT(size == sv.size());
    D_ASSERT(size == sv.capacity());
    D_ASSERT(sv.try_reserve(4));
    D_ASSERT(!sv.try_reserve(5));
    D_ASSERT(size == sv.size());
    D_ASSERT(size == sv.capacity());
}