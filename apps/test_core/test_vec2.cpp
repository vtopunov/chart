#include <core/warnings.h>

D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_signed_unsigned_mismatch)

#include <core/vec2.h>
#include <core/view.h>

#include <vector>


namespace
{
    template<class T>
    void test_view0(span<const T, 2_uz> view, const vec2<T>& vec) noexcept
    {
        static_assert(2u == extent_v<decltype(vec)>);
        static_assert(std::is_same_v<const decltype(view), view_t<decltype(vec)>>);
        static_assert(std::is_same_v<typename vec2<T>::value_type, T>);
        static_assert(std::is_same_v<decltype(vec2<T>::_0), T>);
        static_assert(vec2<T>{}.size() == 2_uz);

        D_ASSERT(view.data() == std::addressof(vec._0));
    }

    template<class T>
    void test_view(const vec2<T>& vec) noexcept
    {
        test_view0<T>(vec, vec);
    }

    template<class T>
    void test_view(vec2<T>& vec) noexcept
    {
        test_view0<T>(vec, vec);
    }
}

void test_vec2() noexcept
{
    {
        static_assert(std::is_same_v<view_t<vec2<int>>, const span<const int, 2_uz>>);
        static_assert(2u == extent_v<vec2<int>>);
    }

    {
        constexpr vec2 v0{ 1, 1 };
        constexpr vec2 v1{ 1, 2 };
        constexpr vec2 v2{ 2, 1 };
        constexpr vec2 v3{ 2, 2 };
        constexpr vec2 v4{ 1, 0 };
        constexpr vec2 v5{ 3, 1 };

        static_assert(std::is_same_v<std::remove_const_t<decltype(v0)>, vec2<int>>);
        static_assert(std::is_same_v<std::remove_const_t<decltype(v0._0)>, int>);

        static_assert(v0.data() == std::addressof(v0._0));
        test_view(v0);

        static_assert(v1.data() == std::addressof(v1._0));
        test_view(v1);

        {
            vec2 v{ 3, 1 };
            test_view(v);
        }

        static_assert(std::is_trivial_v<vec2<int>> && std::is_standard_layout_v<vec2<int>>);

        static_assert(1 == v0._0);
        static_assert(1 == v0._1);
        static_assert(1 == get<0>(v0));
        static_assert(1 == get<1>(v0));
        static_assert(v0 == vec2<int>{ 1, 1 });
        static_assert(!(v0 != vec2<int>{ 1, 1 }));
        static_assert(v0 != v1);
        static_assert(!(v0 == v1));

        static_assert(1 == v1._0);
        static_assert(2 == v1._1);
        static_assert(1 == get<0>(v1));
        static_assert(2 == get<1>(v1));
        static_assert(v1 == vec2<int>{ 1, 2 });
        static_assert(!(v1 != vec2<int>{ 1, 2 }));
        static_assert(v1 != v2);
        static_assert(!(v1 == v2));

        static_assert(2 == v2._0);
        static_assert(1 == v2._1);
        static_assert(2 == get<0>(v2));
        static_assert(1 == get<1>(v2));
        static_assert(v2 == vec2<int>{ 2, 1 });
        static_assert(!(v2 != vec2<int>{ 2, 1 }));
        static_assert(v2 != v0);
        static_assert(!(v2 == v0));

        static_assert(2 * v0 == v0 * 2);
        static_assert(2 * v0 == v3);
        static_assert(v3 / 2 == v0);
        static_assert(v2 + v4 == v5);
        static_assert(v4 + v2 == v5);
        static_assert(v5 - v2 == v4);
        static_assert(v5 - v4 == v2);
        static_assert(-(-v5) == v5);
        static_assert(-(-(-v5)) == -v5);
        static_assert(-(v4 + (-v5)) == v2);
    }

    {
        constexpr auto vec_size = md_narrow<vec2<size_t>>(vec2{ 3, 4 });
        static_assert(std::is_same_v<std::remove_const_t<decltype(vec_size)>, vec2<size_t>>);
        static_assert(std::is_same_v<std::remove_const_t<decltype(vec_size._0)>, size_t>);
        static_assert(vec_size._0 == 3_uz);
        static_assert(vec_size._1 == 4_uz);
        test_view(vec_size);

        constexpr auto vec_ptrdiff = md_narrow<vec2<ptrdiff_t>>(vec_size);
        static_assert(std::is_same_v<std::remove_const_t<decltype(vec_ptrdiff)>, vec2<ptrdiff_t>>);
        static_assert(std::is_same_v<std::remove_const_t<decltype(vec_ptrdiff._0)>, ptrdiff_t>);
        static_assert(vec_ptrdiff._0 == 3);
        static_assert(vec_ptrdiff._1 == 4);
        static_assert(md_narrow<vec2<size_t>>(vec_ptrdiff) == vec_size);
        test_view(vec_ptrdiff);

        const auto stdvec = md_narrow<std::vector<int>>(vec_ptrdiff);
        D_ASSERT(stdvec.size() == 2u);
        D_ASSERT(stdvec[0] == 3);
        D_ASSERT(stdvec[1] == 4);
    }

    {
        constexpr vec2 vvv0
        {
            vec2{ vec2{ 1, 2 }, vec2{ 2, 3 } },
            vec2{ vec2{ 4, 5 }, vec2{ 6, 7 } },
        };

        constexpr auto vvv0u = as_unsigned(vvv0);
        static_assert(vvv0 == as_signed(vvv0));

        constexpr auto mvvv0 = -vvv0;
        static_assert(vvv0 == md_abs(mvvv0));

        {
            constexpr auto min_vvv0 = md_min(vvv0, vvv0);
            static_assert(min_vvv0 == vvv0);
        }

        {
            constexpr auto min_vvv0_mvvv0 = md_min(vvv0, mvvv0);
            static_assert(min_vvv0_mvvv0 == mvvv0);

            constexpr auto min_mvvv0_vvv0 = md_min(mvvv0, vvv0);
            static_assert(min_mvvv0_vvv0 == mvvv0);
        }

        {
            constexpr auto min_vvv0_inv_vvv0 = md_min(vvv0, inverse(vvv0));
            static_assert(min_vvv0_inv_vvv0 == fill_vec2(vvv0._0));

            constexpr auto min_inv_vvv0_vvv0 = md_min(inverse(vvv0), vvv0);
            static_assert(min_inv_vvv0_vvv0 == min_vvv0_inv_vvv0);
        }

        {
            constexpr auto min_vvv0u = md_min(vvv0u, vvv0u);
            static_assert(min_vvv0u == vvv0u);
        }

        {
            constexpr auto min_vvv0_vvv0u = md_min(vvv0, vvv0u);
            static_assert(min_vvv0_vvv0u == vvv0u);
        }

        {
            constexpr auto min_vvv0u_vvv0 = md_min(vvv0u, vvv0);
            static_assert(min_vvv0u_vvv0 == vvv0u);
        }

        {
            constexpr auto max_vvv0 = md_max(vvv0, vvv0);
            static_assert(max_vvv0 == vvv0);
        }

        {
            constexpr auto max_vvv0_mvvv0 = md_max(vvv0, mvvv0);
            static_assert(max_vvv0_mvvv0 == vvv0);

            constexpr auto max_mvvv0_vvv0 = md_max(mvvv0, vvv0);
            static_assert(max_mvvv0_vvv0 == vvv0);
        }

        {
            constexpr auto max_vvv0_inv_vvv0 = md_max(vvv0, inverse(vvv0));
            static_assert(max_vvv0_inv_vvv0 == fill_vec2(vvv0._1));

            constexpr auto max_inv_vvv0_vvv0 = md_max(inverse(vvv0), vvv0);
            static_assert(max_inv_vvv0_vvv0 == max_vvv0_inv_vvv0);
        }

        {
            constexpr auto max_vvv0u = md_max(vvv0u, vvv0u);
            static_assert(max_vvv0u == vvv0u);
        }

        {
            constexpr auto max_vvv0_vvv0u = md_max(vvv0, vvv0u);
            static_assert(max_vvv0_vvv0u == vvv0u);
        }

        {
            constexpr auto max_vvv0u_vvv0 = md_max(vvv0u, vvv0);
            static_assert(max_vvv0u_vvv0 == vvv0u);
        }

        {
            constexpr auto vvvzu = (vvv0u - vvv0u);
            static_assert(vvv0u == md_clamp_cast<decltype(vvv0u)>(vvv0));
            static_assert(vvvzu == md_clamp_cast<decltype(vvv0u)>(-vvv0));
        }

        {
            constexpr auto vvvz_d = fill_vec2(fill_vec2(fill_vec2(0.0)));
            constexpr auto vvve_d = fill_vec2(fill_vec2(fill_vec2(1.0)));
            constexpr auto vvv0_d = 1.0 * vvv0;
            constexpr auto vvv0_p1_d = vvv0 + vvve_d;
            static_assert(vvv0_d == vvv0 * 1.0);
            static_assert(vvv0_d == vvv0 + vvvz_d);
            static_assert(vvv0_d == vvvz_d + vvv0);
            static_assert(vvv0_d == vvv0 - vvvz_d);
            static_assert(vvv0_d == -(vvvz_d - vvv0));
            static_assert(vvv0_d._1._1._0 == 1.0 * vvv0._1._1._0);
            static_assert(vvv0_d + vvve_d == vvv0_p1_d);

            constexpr auto inc_0_9 = fill_vec2(fill_vec2(fill_vec2(0.9)));
            constexpr auto inc_0_1 = fill_vec2(fill_vec2(fill_vec2(0.1)));

            constexpr auto vvv0_0_9 = vvv0 + inc_0_9;
            constexpr auto vvv0_m0_1 = vvv0 - inc_0_1;
            static_assert(std::is_same_v<double, decltype(vvv0_0_9._0._0._0)>);
            static_assert(std::is_same_v<double, decltype(vvv0_m0_1._0._0._0)>);
            static_assert(vvv0_0_9._1._0._1 == vvv0._1._0._1 + 0.9);
            static_assert(vvv0_0_9._1._1._1 == vvv0._1._1._1 + 0.9);
            static_assert(vvv0_m0_1._0._1._1 == vvv0._0._1._1 - 0.1);
            static_assert(vvv0_m0_1._1._1._0 == vvv0._1._1._0 - 0.1);

            D_ASSERT(vvv0 == md_round_to_near(vvv0_0_9, vvv0));
            D_ASSERT(vvv0 == md_round_to_near(vvv0 + inc_0_1, vvv0));
            D_ASSERT(vvv0 == md_round_to_near(vvv0_m0_1, vvv0));

            D_ASSERT(vvv0_p1_d == md_round(vvv0_0_9));
            D_ASSERT(vvv0_d == md_round(vvv0 + inc_0_1));
            D_ASSERT(vvv0_d == md_round(vvv0_m0_1));
            D_ASSERT(vvv0_p1_d == md_round(vvv0 + 0.5 * vvve_d + inc_0_1));
            D_ASSERT(vvv0_d == md_round(vvv0 + 0.5 * vvve_d - inc_0_1));

            static_assert(vvv0_d == md_narrow<decltype(vvv0_d)>(vvv0));
            static_assert(vvv0_d == md_numeric_cast<decltype(vvv0_d)>(vvv0));
            static_assert(vvv0_d == md_trunc_cast<decltype(vvv0_d)>(vvv0));

            D_ASSERT(vvv0 == md_trunc_cast<decltype(vvv0)>(vvv0_d));
            D_ASSERT(vvv0 == md_trunc_cast<decltype(vvv0)>(vvv0 + inc_0_9));
            D_ASSERT(vvv0 == -md_trunc_cast<decltype(vvv0)>(-(vvv0 + inc_0_9)));
        }
    }

    {
        const errno_holder hold_errno{};

        constexpr vec2 vvv0
        {
            vec2{ vec2{ 1., 2. }, vec2{ 2., 3. } },
            vec2{ vec2{ 4., 5. }, vec2{ 6., 7. } },
        };

        auto inf_vvv0 = vvv0;
        inf_vvv0._1._1._1 = numeric_inf_v<>;

        auto denorm_vvv0 = vvv0;
        const auto denorm_zero = std::nextafter(0.0, 1.0);
        D_ASSERT(!std::isnormal(denorm_zero));
        denorm_vvv0._1._1._1 = denorm_zero;

        D_ASSERT(md_isfinite(vvv0));
        D_ASSERT(!md_isfinite(inf_vvv0));
        D_ASSERT(md_isnormal(vvv0));
        D_ASSERT(!md_isnormal(inf_vvv0));

        D_ASSERT(md_isfinite(denorm_vvv0));
        D_ASSERT(!md_isnormal(denorm_vvv0));
    }

    {
        struct derived_vec2f64 : vec2<double>
        {};

        struct derived_vec22f64 : vec2<derived_vec2f64>
        {};

        struct derived_vec222f64 : vec2<derived_vec22f64>
        {};

        constexpr derived_vec222f64 vvv0
        {
            derived_vec22f64{ derived_vec2f64{ 1., 2. }, derived_vec2f64{ 2., 3. } },
            derived_vec22f64{ derived_vec2f64{ 4., 5. }, derived_vec2f64{ 6., 7. } },
        };

        {
            constexpr auto min_vvv0 = md_min(vvv0, vvv0);
            static_assert(min_vvv0 == -(-vvv0));
        }

        {
            constexpr auto min_vvv0 = md_min(vvv0, -vvv0);
            static_assert(min_vvv0 == -vvv0);
        }
    }


    {
        vec2<int> zvi{ 0, 0 };
        constexpr vec2<int> vi56{ 5, 6 };
        (zvi += vi56) += vi56;
        D_ASSERT(2 * vi56 == zvi);
        (zvi -= vi56) -= vi56;
        D_ASSERT(0 == zvi._0);
        D_ASSERT(0 == zvi._1);
        zvi += vi56;
        D_ASSERT(vi56 == zvi);
        zvi *= 3;
        D_ASSERT(3 * vi56 == zvi);
        zvi /= 3;
        D_ASSERT(vi56 == zvi);
    }
}


D_WARNING_POP