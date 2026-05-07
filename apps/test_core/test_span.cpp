#include <vector>
#include <random>

#include <core/span.h>


namespace
{
    void test_span_vector_property() noexcept
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
    }

    void test_span_to_cspan() noexcept
    {
        std::vector<int> v{ 1, 2, 3, 4, 5 };
        span<int> sv{ v };

        span<const int> csv{ sv };
        D_ASSERT(csv.data() == sv.data());
        D_ASSERT(csv.size() == sv.size());
        static_assert(!std::is_same_v<decltype(csv.front()), decltype(sv.front())>);
        static_assert(std::is_same_v<decltype(csv.front()), decltype(std::as_const(sv.front()))>);

        csv = sv;
        D_ASSERT(csv.data() == sv.data());
        D_ASSERT(csv.size() == sv.size());
    }

    void test_cvector_to_cspan() noexcept
    {
        const std::vector<int> cv{ 1, 2, 3, 4, 5 };

        span<const int> csv{ cv };
        D_ASSERT(csv.data() == cv.data());
        D_ASSERT(csv.size() == cv.size());

        const auto cv2 = cv;
        csv = cv2;
        D_ASSERT(csv.data() == cv2.data());
        D_ASSERT(csv.size() == cv2.size());
    }

    void test_ccarray_to_cspan() noexcept
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

    void test_ccarray_to_cspanarray() noexcept
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

        span<const int> csv{ csv3 };
        D_ASSERT(csv.data() == std::data(cv3));
        D_ASSERT(csv.size() == std::size(cv3));

        csv = csv4;
        D_ASSERT(csv.data() == std::data(cv4));
        D_ASSERT(csv.size() == std::size(cv4));
    }

    void test_carray_to_spanarray_cspanarray() noexcept
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

        {
            span<int> sv{ sv3 };
            D_ASSERT(sv.data() == std::data(v3));
            D_ASSERT(sv.size() == std::size(v3));

            sv = sv4;
            D_ASSERT(sv.data() == std::data(v4));
            D_ASSERT(sv.size() == std::size(v4));
        }

        {
            span<const int> csv{ sv3 };
            D_ASSERT(csv.data() == std::data(v3));
            D_ASSERT(csv.size() == std::size(v3));

            csv = sv4;
            D_ASSERT(csv.data() == std::data(v4));
            D_ASSERT(csv.size() == std::size(v4));
        }

        {
            constexpr size_t extent = 3u;

            {
                {
                    span<int, extent> sve4{ v4 };
                    D_ASSERT(sve4.data() == std::data(v4));
                    D_ASSERT(extent == sve4.size());
                }

                {
                    span<int, extent> sve4{ sv4 };
                    D_ASSERT(sve4.data() == std::data(v4));
                    D_ASSERT(extent == sve4.size());
                }

                {
                    span<const int, extent> csve4{ v4 };
                    D_ASSERT(csve4.data() == std::data(v4));
                    D_ASSERT(extent == csve4.size());
                }

                {
                    span<const int, extent> csve4{ sv4 };
                    D_ASSERT(csve4.data() == std::data(v4));
                    D_ASSERT(extent == csve4.size());
                }
            }

            {
                {
                    span<int, extent> sve4;
                    sve4 = v4;
                    D_ASSERT(sve4.data() == std::data(v4));
                    D_ASSERT(extent == sve4.size());
                }

                {
                    span<int, extent> sve4;
                    sve4 = sv4;
                    D_ASSERT(sve4.data() == std::data(v4));
                    D_ASSERT(extent == sve4.size());
                }

                {
                    span<const int, extent> csve4;
                    csve4 = v4;
                    D_ASSERT(csve4.data() == std::data(v4));
                    D_ASSERT(extent == csve4.size());
                }

                {
                    span<const int, extent> csve4;
                    csve4 = sv4;
                    D_ASSERT(csve4.data() == std::data(v4));
                    D_ASSERT(extent == csve4.size());
                }
            }
        }
    }

    void test_span_deduction_guides() noexcept
    {
        std::vector<int> v{ 1, 2, 3, 4, 5 };
        span<int> sv{ v };
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

    void test_span_algorithm() noexcept
    {
        static constexpr int cv8[]{ 1, 2, 3, 4, 5, 6, 7, 8 };
        static_assert(0u == (std::size(cv8) % 2));

        std::vector<int> v(std::size(cv8), -1);
        D_ASSERT(v.size() == std::size(cv8));
        D_ASSERT(v[0] != cv8[0]);
        D_ASSERT(v[0] == -1);

        copy(make_cspan(cv8), v.begin());
        D_ASSERT(v.size() == std::size(cv8));
        D_ASSERT(!memcmp(v.data(), std::data(cv8), std::size(cv8)));

        constexpr auto half_size = std::size(cv8) / 2u;
        copy(make_span(std::as_const(v)).last(half_size), v.begin());
        D_ASSERT(!memcmp(v.data(), v.data() + half_size, half_size));
        v.assign(std::cbegin(cv8), std::cend(cv8));
        D_ASSERT(v.size() == std::size(cv8));
        D_ASSERT(!memcmp(v.data(), std::data(cv8), std::size(cv8)));

        copy(make_cspan(v).last(half_size), v.begin());
        D_ASSERT(!memcmp(v.data(), v.data() + half_size, half_size));
        v.assign(std::cbegin(cv8), std::cend(cv8));

        {
            auto v2 = v;
            v.assign(v.size(), -1);
            copy(make_cspan(v2), v.begin());
            D_ASSERT(v.size() == std::size(cv8));
            D_ASSERT(!memcmp(v.data(), std::data(cv8), std::size(cv8)));
        }

        {
            std::random_device entropy{};
            std::mt19937 random_engine{ entropy() };
            auto v2 = v;
            for (int i = 0; i < 100; ++i)
            {
                const auto value = as_signed(random_engine());
                v.assign(v.size(), value);
                fill(make_span(v2), value);
                D_ASSERT(v.size() == v2.size());
                D_ASSERT(!memcmp(v.data(), v2.data(), v.size()));
            }
        }

        v.assign(std::cbegin(cv8), std::cend(cv8));
        for (size_t i = 0; i < v.size(); ++i)
        {
            D_ASSERT(find_n(make_cspan(cv8), cv8[i]) == i);
            D_ASSERT(find_n(make_cspan(v), v[i]) == i);

            {
                const auto i_it = v.cbegin() + i;
                D_ASSERT(i_it < v.cend());
                D_ASSERT(i_it == std::find(v.cbegin(), v.cend(), *i_it));
                D_ASSERT(v.cend() == std::find(std::next(i_it), v.cend(), *i_it));
                D_ASSERT(v.size() == find_n(make_cspan(v), *i_it, u_next(i)));
            }

            {
                const auto sp_v = make_cspan(v);
                const auto i_it = sp_v.cbegin() + i;
                D_ASSERT(i_it < sp_v.cend());
                D_ASSERT(i_it == std::find(sp_v.cbegin(), sp_v.cend(), *i_it));
                D_ASSERT(find_n(sp_v, *i_it) == i);

                {
                    const auto no_i_sp_v = sp_v.subspan(u_next(i));
                    D_ASSERT(i_it < no_i_sp_v.cend());
                    D_ASSERT(no_i_sp_v.cend() == std::find(no_i_sp_v.cbegin(), no_i_sp_v.cend(), *i_it));
                    D_ASSERT(no_i_sp_v.size() == find_n(no_i_sp_v, *i_it));
                }
            }
        }
    }
}

void test_span() noexcept
{
    static_assert(dynamic_extent == numeric_max_v<decltype(dynamic_extent)>);
    static_assert(std::is_same_v<span<const int>, decl_view_type_t<span<int>>>);
    static_assert(std::is_same_v<span<const int>, decl_view_type_t<span<const int>>>);
    static_assert(std::is_same_v<nullmem_t, decl_null_type_t<span<int>>>);
    static_assert(std::is_same_v<nullmem_t, decl_null_type_t<span<const int>>>);

    test_span_vector_property();
    test_span_to_cspan();
    test_cvector_to_cspan();
    test_ccarray_to_cspan();
    test_ccarray_to_cspanarray();
    test_carray_to_spanarray_cspanarray();
    test_span_deduction_guides();
    test_span_algorithm();
}