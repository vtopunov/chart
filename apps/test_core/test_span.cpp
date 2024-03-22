#include <vector>

#include <core/span.h>


void test_span() noexcept
{
    static_assert(dynamic_extent == numeric_max_v<size_t>);
    static_assert(std::is_same_v<span<const int>, decl_view_type_t<span<int>>>);
    static_assert(std::is_same_v<span<const int>, decl_view_type_t<span<const int>>>);
    static_assert(std::is_same_v<nullmem_t, decl_null_type_t<span<int>>>);
    static_assert(std::is_same_v<nullmem_t, decl_null_type_t<span<const int>>>);

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

        const auto cv2 = cv;
        csv = cv2;
        D_ASSERT(csv.data() == cv2.data());
        D_ASSERT(csv.size() == cv2.size());
    }

    {
        constexpr int cv3[]{ 1, 2, 3 };
        span<const int> csv{ cv3 };
        D_ASSERT(csv.data() == std::data(cv3));
        D_ASSERT(csv.size() == std::size(cv3));

        constexpr int cv4[]{ 1, 2, 3, 4 };
        csv = cv4;
        D_ASSERT(csv.data() == std::data(cv4));
        D_ASSERT(csv.size() == std::size(cv4));
    }

    {
        constexpr int cv3[]{ 1, 2, 3 };
        span<const int, 3u> csv3{ cv3 };
        D_ASSERT(csv3.data() == std::data(cv3));
        D_ASSERT(csv3.size() == std::size(cv3));

        constexpr int cv4[]{ 1, 2, 3, 4 };
        span<const int, 4u> csv4;
        csv4 = cv4;
        D_ASSERT(csv4.data() == std::data(cv4));
        D_ASSERT(csv4.size() == std::size(cv4));
    }

    {
        int v3[]{ 1, 2, 3 };
        span<const int, 3u> csv3{ v3 };
        span<int, 3u> sv3{ v3 };
        D_ASSERT(csv3.data() == std::data(v3));
        D_ASSERT(csv3.size() == std::size(v3));
        D_ASSERT(sv3.data() == std::data(v3));
        D_ASSERT(sv3.size() == std::size(v3));

        int v4[]{ 1, 2, 3, 4 };
        span<const int, 4u> csv4;
        span<int, 4u> sv4;
        csv4 = v4;
        sv4 = v4;
        D_ASSERT(csv4.data() == std::data(v4));
        D_ASSERT(csv4.size() == std::size(v4));
        D_ASSERT(sv4.data() == std::data(v4));
        D_ASSERT(sv4.size() == std::size(v4));
    }

    {
        int v3[]{ 1, 2, 3 };
        int v4[]{ 1, 2, 3, 4 };
        constexpr int cv3[]{ 1, 2, 3 };
        constexpr int cv4[]{ 1, 2, 3, 4 };
        span sv3{ v3 };
        span sv4{ v4 };
        span csv3{ cv3 };
        span csv4{ cv4 };
        static_assert(std::is_same_v<typename decltype(sv3)::value_type, int>);
        static_assert(decltype(sv3)::extent == 3);
        static_assert(std::is_same_v<typename decltype(csv3)::value_type, const int>);
        static_assert(decltype(csv3)::extent == 3);
        static_assert(std::is_same_v<typename decltype(sv4)::value_type, int>);
        static_assert(decltype(sv4)::extent == 4);
        static_assert(std::is_same_v<typename decltype(csv4)::value_type, const int>);
        static_assert(decltype(csv4)::extent == 4);
        D_ASSERT(sv3.data() == std::data(v3));
        D_ASSERT(sv3.size() == std::size(v3));
        D_ASSERT(csv3.data() == std::data(cv3));
        D_ASSERT(csv3.size() == std::size(cv3));
        D_ASSERT(sv4.data() == std::data(v4));
        D_ASSERT(sv4.size() == std::size(v4));
        D_ASSERT(csv4.data() == std::data(cv4));
        D_ASSERT(csv4.size() == std::size(cv4));

        const auto sv3c = sv3;
        const auto csv3c = csv3;
        D_ASSERT(sv3c == sv3);
        D_ASSERT(csv3c == csv3);

        decltype(sv) svv3c{ v3 };
        D_ASSERT(svv3c != sv);
    }
}