#include <core/invoke.h>


namespace
{
    namespace private_detail_test_is_invocable
    {
        template<int>
        struct arg {};

        template<int... ids>
        struct functor_for
        {
            constexpr void operator () (arg<ids>...) const noexcept
            {}
        };
    }

    void test_is_invocable_decl() noexcept
    {
        using namespace private_detail_test_is_invocable;

        static_assert(is_invalid_invoke_result_v<decltype(invalid_invoke)>);
        static_assert(is_invalid_invoke_result_v<decltype(no_invocable)>);
        static_assert(is_invalid_invoke_result_v<decltype(no_convertible)>);

        static_assert(is_invocable_decl_v<functor_for<0>, arg<0>>);
        static_assert(!is_invocable_decl_v<functor_for<0>, arg<1>>);
        static_assert(is_invocable_decl_v<functor_for<0, 1>, arg<0>, arg<1>>);
        static_assert(!is_invocable_decl_v<functor_for<0, 1>, arg<0>, arg<0>>);
        static_assert(!is_invocable_decl_v<functor_for<0, 1>, arg<1>, arg<0>>);
        static_assert(!is_invocable_decl_v<functor_for<0, 1>, arg<1>, arg<1>>);
        D_ASSERT(!errno);
    }

    namespace private_detail_invoke_if_exist
    {
        template<int>
        struct S { int i; };

        template<int i>
        constexpr S<i> S_v{};

        struct
        {
            int ncall_0{ 0 };
            int ncall_1{ 0 };

            S<2> operator () (S<0>) noexcept { return { ++ncall_0 }; };
            S<3> operator () (S<1>) noexcept { return { ++ncall_1 }; };
        } f0{};

        int ncall_f1{ 0 };
        int ncall_f2{ 0 };

        S<4> f1(S<0>) noexcept { return { ++ncall_f1 }; };
        S<5> f2(S<1>) noexcept { return { ++ncall_f2 }; };
    }

    void test_invoke_if_exist() noexcept
    {
        using namespace private_detail_invoke_if_exist;
        using private_detail_invoke_if_exist::S;

        {
            D_ASSERT(0 == f0.ncall_0 && 0 == f0.ncall_1);
            const auto r_f0_0 = invoke_if_exist(f0, S_v<0>);
            static_assert(std::is_same_v<decltype(r_f0_0), const S<2>>);
            D_ASSERT(1 == f0.ncall_0 && 0 == f0.ncall_1);
            D_ASSERT(r_f0_0.i == 1);

            const auto r_f0_0_next = invoke_if_exist(f0, S_v<0>);
            D_ASSERT(2 == f0.ncall_0 && 0 == f0.ncall_1);
            D_ASSERT(r_f0_0_next.i == 2);

            const auto r_f0_1 = invoke_if_exist(f0, S_v<1>);
            static_assert(std::is_same_v<decltype(r_f0_1), const S<3>>);
            D_ASSERT(2 == f0.ncall_0 && 1 == f0.ncall_1);
            D_ASSERT(r_f0_1.i == 1);

            const auto r_f0_2 = invoke_if_exist(f0, S_v<2>);
            static_assert(std::is_same_v<decltype(r_f0_2), const no_invocable_t>);
            D_ASSERT(2 == f0.ncall_0 && 1 == f0.ncall_1);

            const auto r_f0_01 = invoke_if_exist(f0, S_v<0>, S_v<1>);
            static_assert(std::is_same_v<decltype(r_f0_01), const no_invocable_t>);
            D_ASSERT(2 == f0.ncall_0 && 1 == f0.ncall_1);
        }

        {
            D_ASSERT(0 == ncall_f1);
            const auto r_f1_0 = invoke_if_exist(f1, S_v<0>);
            static_assert(std::is_same_v<decltype(r_f1_0), const S<4>>);
            D_ASSERT(1 == ncall_f1);
            D_ASSERT(r_f1_0.i == 1);

            const auto r_f1_1 = invoke_if_exist(f1, S_v<1>);
            static_assert(std::is_same_v<decltype(r_f1_1), const no_invocable_t>);
            D_ASSERT(1 == ncall_f1);
        }

        {
            D_ASSERT(0 == ncall_f2);
            const auto r_f2_1 = invoke_if_exist(f2, S_v<1>);
            static_assert(std::is_same_v<decltype(r_f2_1), const S<5>>);
            D_ASSERT(1 == ncall_f1);
            D_ASSERT(1 == ncall_f2);
            D_ASSERT(r_f2_1.i == 1);

            const auto r_f2_0 = invoke_if_exist(f2, S_v<0>);
            static_assert(std::is_same_v<decltype(r_f2_0), const no_invocable_t>);
            D_ASSERT(1 == ncall_f1);
            D_ASSERT(1 == ncall_f2);
        }
    }
}

void test_invoke() noexcept
{
    test_is_invocable_decl();
    test_invoke_if_exist();
}