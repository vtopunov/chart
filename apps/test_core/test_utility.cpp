#include <core/utility.h>


namespace
{
    void test_value_type() noexcept
    {
        {
            struct my_vector0
            {
                struct value_type
                {};
            };

            static_assert(std::is_same_v<decl_value_type_t<my_vector0>, typename my_vector0::value_type>);
            static_assert(is_detected_v<value_type_t, my_vector0>);
            static_assert(std::is_same_v<value_type_t<my_vector0>, typename my_vector0::value_type>);
            static_assert(!has_std_data_pointer<my_vector0>::value);
            static_assert(!has_std_data_pointer<const my_vector0&>::value);
        }

        {
            struct my_vector1
            {
                struct value_type_impl
                {};

                constexpr const value_type_impl* data() const noexcept
                {
                    return nullptr;
                }
            };

            static_assert(std::is_same_v<decl_std_data_pointer_t<my_vector1>, const typename my_vector1::value_type_impl*>);
            static_assert(is_detected_v<value_type_t, my_vector1>);
            static_assert(std::is_same_v<value_type_t<my_vector1>, const typename my_vector1::value_type_impl>);
            static_assert(has_std_data_pointer<my_vector1>::value);
        }


        {
            struct my_vector2
            {
                struct value_type_impl
                {};

                constexpr const value_type_impl* data() const noexcept
                {
                    return nullptr;
                }

                constexpr value_type_impl* data() noexcept
                {
                    return nullptr;
                }
            };

            static_assert(std::is_same_v<decl_std_data_pointer_t<my_vector2>, typename my_vector2::value_type_impl*>);
            static_assert(std::is_same_v<decl_std_data_pointer_t<const my_vector2>, const typename my_vector2::value_type_impl*>);
            static_assert(is_detected_v<value_type_t, my_vector2>);
            static_assert(std::is_same_v<value_type_t<my_vector2>, typename my_vector2::value_type_impl>);
            static_assert(std::is_same_v<value_type_t<const my_vector2>, const typename my_vector2::value_type_impl>);
            static_assert(has_std_data_pointer<my_vector2>::value);
        }

        {
            struct my_vector3
            {
                struct value_type
                {};

                struct data_value_type
                {};

                constexpr const data_value_type* data() const noexcept
                {
                    return nullptr;
                }

                constexpr data_value_type* data() noexcept
                {
                    return nullptr;
                }
            };

            static_assert(std::is_same_v<decl_std_data_pointer_t<my_vector3>, typename my_vector3::data_value_type*>);
            static_assert(std::is_same_v<decl_std_data_pointer_t<const my_vector3>, const typename my_vector3::data_value_type*>);
            static_assert(std::is_same_v<typename std_data_value_type_type<my_vector3>::type, typename my_vector3::data_value_type>);
            static_assert(is_detected_v<value_type_t, my_vector3>);
            static_assert(std::is_same_v<value_type_t<my_vector3>, typename my_vector3::value_type>);
            static_assert(std::is_same_v<value_type_t<const my_vector3>, typename my_vector3::value_type>);
            static_assert(has_std_data_pointer<my_vector3>::value);
            static_assert(has_std_data_pointer<const my_vector3&>::value);
        }

        {
            struct my_vector4
            {
                constexpr const dummy data() const noexcept
                {
                    return {};
                }

                constexpr dummy data() noexcept
                {
                    return {};
                }
            };

            static_assert(!is_detected_v<decl_std_data_pointer_t, my_vector4>);
            static_assert(!is_detected_v<decl_value_type_t, my_vector4>);
            static_assert(!is_detected_v<value_type_t, my_vector4>);
            static_assert(!has_std_data_pointer<my_vector4>::value);
            static_assert(!is_detected_v<decl_std_data_pointer_t, const my_vector4>);
            static_assert(!is_detected_v<decl_value_type_t, const my_vector4>);
            static_assert(!is_detected_v<value_type_t, const my_vector4>);
            static_assert(!has_std_data_pointer<const my_vector4>::value);
        }

        {
            static_assert(has_std_data_pointer<int[3]>::value);
            static_assert(std::is_same_v<value_type_t<int[3]>, int>);
        }

        D_ASSERT(!errno);
    }

    namespace private_detail_test_size_type
    {
        template<size_t aling>
        void test_size_aling() noexcept
        {
            constexpr auto stop = 0x10000_uz;

            static_assert(size_align<aling>(0_uz) == 0_uz);

            for (size_t aligned = 0; aligned <= stop; )
            {
                const auto next_aligned = aligned + aling;

                for (size_t unaligned = aligned + 1_uz; unaligned <= next_aligned; ++unaligned)
                {
                    D_ASSERT(size_align<aling>(unaligned) == next_aligned);
                }

                aligned = next_aligned;
            }
        }
    }

    void test_size_type() noexcept
    {
        using namespace private_detail_test_size_type;

        test_size_aling<1_uz>();
        test_size_aling<2_uz>();
        test_size_aling<4_uz>();
        test_size_aling<8_uz>();
        test_size_aling<16_uz>();
    }

    namespace private_detail_test_u_swap
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

    void test_u_swap() noexcept
    {
        using namespace private_detail_test_u_swap;

        size_t ctx{ 0u };
        S0 s0{ ctx };
        S1 s1{ ctx };

        {
            ctx = {};
            u_swap(s0, s0);
            D_ASSERT(swap_S0_S0 == ctx);
        }

        {
            ctx = {};
            u_swap(s1, s0);
            D_ASSERT(swap_S1_S0 == ctx);
        }

        {
            ctx = {};
            u_swap(s0, s1);
            D_ASSERT(swap_S1_S0 == ctx);
        }

        {
            ctx = {};
            u_swap(s1, s1);
            D_ASSERT(swap_S1_S1 == ctx);
        }
    }

    void test_u_minmax() noexcept
    {
        constexpr auto min_d = u_min(0, 0.1);
        constexpr auto max_d = u_max(0, 0.1);
        constexpr auto min_u = u_min(-1, 5u);
        constexpr auto max_u = u_max(-1, 5u);

        static_assert(0.0 == min_d);
        static_assert(0.1 == max_d);
        static_assert(5u == min_u);
        static_assert((0u - 1u) == max_u);
        static_assert(std::is_same_v<const double, decltype(min_d)>);
        static_assert(std::is_same_v<const double, decltype(max_d)>);
        static_assert(std::is_same_v<const unsigned, decltype(min_u)>);
        static_assert(std::is_same_v<const unsigned, decltype(max_u)>);

        D_ASSERT(!errno);
    }
}

void test_utility() noexcept
{
    test_value_type();
    test_size_type();
    test_u_swap();
    test_u_minmax();
}