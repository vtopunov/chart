#include <vector>

#include <core/span.h>
#include <core/utility.h>

void test_span() noexcept
{
    std::vector<int> v{ 1, 2, 3, 4, 5 };
    span<int> sv{ v };
    D_ASSERT(v.size() > 0);
    D_ASSERT(sv.data() == v.data());
    D_ASSERT(sv.size() == v.size());
    D_ASSERT(sv.begin() == sv.data());
    D_ASSERT(sv.end() == sv.data() + sv.size());
    D_ASSERT(sv.cbegin() == sv.begin());
    D_ASSERT(sv.cend() == sv.end());
    D_ASSERT(sv.front() == v.front());
    D_ASSERT(sv.back() == v.back());

    {
        span<const int> csv{ sv };
        D_ASSERT(csv.data() == sv.data());
        D_ASSERT(csv.size() == sv.size());
        static_assert(!std::is_same_v<decltype(csv.front()), decltype(sv.front())>);
        static_assert(std::is_same_v<decltype(csv.front()), decltype(std::as_const(sv.front()))>);
        
        csv = sv;
        D_ASSERT(csv.data() == sv.data());
        D_ASSERT(csv.size() == sv.size());
    }

    {
        const std::vector<int> cv{ v };
        span<const int> csv{ cv };
        D_ASSERT(csv.data() == cv.data());
        D_ASSERT(csv.size() == cv.size());

        csv = cv;
        D_ASSERT(csv.data() == cv.data());
        D_ASSERT(csv.size() == cv.size());
    }

    D_ASSERT(!errno);
}