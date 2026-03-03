#include <core/utility.h>

#include <array>

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
            static_assert(std::is_same_v<value_type_t<const my_vector0>, const typename my_vector0::value_type>);
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
            static_assert(is_detected_v<value_type_t, my_vector3>);
            static_assert(std::is_same_v<value_type_t<my_vector3>, typename my_vector3::data_value_type>);
            static_assert(std::is_same_v<value_type_t<const my_vector3>, const typename my_vector3::data_value_type>);
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

                for (size_t unaligned = aligned + 1u; unaligned <= next_aligned; ++unaligned)
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

    namespace private_detail_test_test_no_unique_address
    {
        struct OverAlign
        {
            void* data;
            dummy test;
        };

        struct OverAlignSmall
        {
            void* data;
            D_NO_UNIQUE_ADDRESS dummy test;
        };
    }

    void test_test_no_unique_address() noexcept
    {
        using namespace private_detail_test_test_no_unique_address;

        static_assert(sizeof(OverAlignSmall) < sizeof(OverAlign));
        static_assert(test_no_unique_address(&OverAlignSmall::test));
        static_assert(!test_no_unique_address(&OverAlign::test));
       
        static_assert(has_no_unique_address_v<dummy>);
        static_assert(!has_no_unique_address_v<errno_holder>);

        D_ASSERT(sizeof(OverAlignSmall) < sizeof(OverAlign));
        D_ASSERT(test_no_unique_address(&OverAlignSmall::test));
        D_ASSERT(!test_no_unique_address(&OverAlign::test));
        

        D_ASSERT(!errno);
    }

    void test_ceil_div() noexcept
    {
        const errno_holder hold_errno{};

        for (size_t num = 0; num < 100; ++num)
        {
            const double numf = static_cast<double>(num);

            for (size_t den = 1; den < 100; ++den)
            {
                const auto divi = ceil_div(num, den);
                const auto divf = std::ceil(numf / den);
                const auto divfi = static_cast<decltype(divf)>(divi);
                D_ASSERT(is_eqfp(divfi, divf));
            }
        }
    }

    namespace private_detail_test_test_cdata
    {
        namespace id_data
        {
            constexpr char cdatac[] = "cdatac";
            constexpr char cdata[] = "cdata";
            constexpr char datac[] = "datac";
            constexpr char data[] = "data";
        }

        struct vector0
        {
            const char* data() const noexcept
            {
                return id_data::datac;
            }

            const char* data() noexcept
            {
                return id_data::data;
            }
        };

        struct vector1
        {
            const char* cdata() const noexcept
            {
                return id_data::cdatac;
            }

            const char* cdata() noexcept
            {
                return id_data::cdata;
            }

            const char* data() const noexcept
            {
                return id_data::datac;
            }

            const char* data() noexcept
            {
                return id_data::data;
            }
        };
    }

    void test_cdata() noexcept
    {
        using namespace private_detail_test_test_cdata;

        char mut_data[sizeof(id_data::cdatac)];
        memcpy(mut_data, id_data::cdatac, sizeof(mut_data));
       
        static_assert(std::is_same_v<const char, std::remove_pointer_t<decltype(cdata(mut_data))> > );
        D_ASSERT(mut_data == cdata(mut_data));

        vector0 v0;
        const vector0 cv0;
        vector1 v1;
        const vector1 cv1;

        D_ASSERT(id_data::datac == cdata(v0));
        D_ASSERT(id_data::datac == cdata(cv0));
        D_ASSERT(id_data::cdatac == cdata(v1));
        D_ASSERT(id_data::cdatac == cdata(cv1));
    }

    void test_construct_at() noexcept
    {
        std::array<int, 2u> arr{};
        int value{};
        D_ASSERT(0 == arr[0]);
        ::construct_at(std::addressof(arr), 123);
        D_ASSERT(123 == arr[0]);
        D_ASSERT(0 == arr[1]);
        D_ASSERT(0 == value);
        ::construct_at(std::addressof(value), 123.33);
        D_ASSERT(123 == value);
    }

    void test_errno_holder_and_destroy() noexcept
    {
        const errno_holder hold_errno{};

        {
            errno = 32;
            errno_holder test_hold_errno{};

            errno = 33;
            D_ASSERT(33 == errno);
            destroy(test_hold_errno);
            D_ASSERT(32 == errno);

            errno = 33;
            D_ASSERT(33 == errno);
            destroy_at(std::addressof(test_hold_errno));
            D_ASSERT(32 == errno);
        }
    }
}

void test_utility() noexcept
{
    test_value_type();
    test_size_type();
    test_u_swap();
    test_u_minmax();
    test_test_no_unique_address();
    test_ceil_div();
    test_cdata();
    test_construct_at();
    test_errno_holder_and_destroy();
}