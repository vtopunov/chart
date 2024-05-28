#include <core/type_traits.h>

#include <vector>
#include <span>
#include <cerrno>


namespace
{
    namespace private_detail_test_ordered_overload
    {
        struct with_method
        {
            constexpr void method() const noexcept
            {}
        };

        struct without_method {};

        template<class T>
        constexpr auto call_method_if_exist(T& tested) noexcept -> decltype((std::declval<T&>().method(), true))
        {
            tested.method();
            return true;
        }

        constexpr bool call_method_if_exist(no_overload) noexcept
        {
            return false;
        }
    }

    void test_ordered_overload() noexcept
    {
        using namespace private_detail_test_ordered_overload;

        constexpr with_method swt{};
        constexpr without_method swot{};

        static_assert(call_method_if_exist(swt));
        static_assert(!call_method_if_exist(swot));

        D_ASSERT(!errno);
    }

    namespace private_detail_call_if_exist
    {
        template<int i>
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

    void test_call_if_exist() noexcept
    {
        using namespace private_detail_call_if_exist;
        using private_detail_call_if_exist::S;

        {
            D_ASSERT(0 == f0.ncall_0 && 0 == f0.ncall_1);
            const auto r_f0_0 = call_if_exist(f0, S_v<0>);
            static_assert(std::is_same_v<decltype(r_f0_0), const S<2>>);
            D_ASSERT(1 == f0.ncall_0 && 0 == f0.ncall_1);
            D_ASSERT(r_f0_0.i == 1);

            const auto r_f0_0_next = call_if_exist(f0, S_v<0>);
            D_ASSERT(2 == f0.ncall_0 && 0 == f0.ncall_1);
            D_ASSERT(r_f0_0_next.i == 2);

            const auto r_f0_1 = call_if_exist(f0, S_v<1>);
            static_assert(std::is_same_v<decltype(r_f0_1), const S<3>>);
            D_ASSERT(2 == f0.ncall_0 && 1 == f0.ncall_1);
            D_ASSERT(r_f0_1.i == 1);

            const auto r_f0_2 = call_if_exist(f0, S_v<2>);
            static_assert(std::is_same_v<decltype(r_f0_2), const no_overload>);
            D_ASSERT(2 == f0.ncall_0 && 1 == f0.ncall_1);

            const auto r_f0_01 = call_if_exist(f0, S_v<0>, S_v<1>);
            static_assert(std::is_same_v<decltype(r_f0_01), const no_overload>);
            D_ASSERT(2 == f0.ncall_0 && 1 == f0.ncall_1);
        }

        {
            D_ASSERT(0 == ncall_f1);
            const auto r_f1_0 = call_if_exist(f1, S_v<0>);
            static_assert(std::is_same_v<decltype(r_f1_0), const S<4>>);
            D_ASSERT(1 == ncall_f1);
            D_ASSERT(r_f1_0.i == 1);

            const auto r_f1_1 = call_if_exist(f1, S_v<1>);
            static_assert(std::is_same_v<decltype(r_f1_1), const no_overload>);
            D_ASSERT(1 == ncall_f1);
        }

        {
            D_ASSERT(0 == ncall_f2);
            const auto r_f2_1 = call_if_exist(f2, S_v<1>);
            static_assert(std::is_same_v<decltype(r_f2_1), const S<5>>);
            D_ASSERT(1 == ncall_f1);
            D_ASSERT(1 == ncall_f2);
            D_ASSERT(r_f2_1.i == 1);

            const auto r_f2_0 = call_if_exist(f2, S_v<0>);
            static_assert(std::is_same_v<decltype(r_f2_0), const no_overload>);
            D_ASSERT(1 == ncall_f1);
            D_ASSERT(1 == ncall_f2);
        }
    }

    namespace private_detail_test_member_detector
    {
        template<class T>
        using copy_assign_t = decltype(std::declval<T&>() = std::declval<const T&>());

        struct with_cp {};
        struct without_cp { void operator=(const without_cp&) = delete; };

        struct with_void_cp
        {
            constexpr void operator=(const with_void_cp&) noexcept
            {}
        };


        template<class Ex>
        struct with_ex_cp
        {
            constexpr Ex operator=(const with_ex_cp&) noexcept
            {
                return {};
            }
        };

        template<class It>
        using decl_difference_t = typename It::difference_type;

        template<class It>
        using difference_t = detected_or_t<std::ptrdiff_t, decl_difference_t, It>;

        template<class DT>
        struct with_decl_difference { using difference_type = DT; };
        struct without_decl_difference {};

        template <class Expected, template<class...> class Op, class... Args>
        using is_detected_exact_deprecated = std::is_same<Expected, detected_t<Op, Args...>>;

        template <class Expected, template<class...> class Op, class... Args>
        constexpr auto is_detected_exact_deprecated_v = is_detected_exact_deprecated<Expected, Op, Args...>::value;
    }

    void test_member_detector() noexcept
    {
        using namespace private_detail_test_member_detector;

        static_assert(!std::is_convertible_v<nonesuch, nonesuch>);

        static_assert(is_detected_v<copy_assign_t, with_cp>);
        static_assert(!is_detected_v<copy_assign_t, without_cp>);

        static_assert(is_detected_exact_v<with_cp&, copy_assign_t, with_cp>);
        static_assert(is_detected_exact_v<void, copy_assign_t, with_void_cp>);
        static_assert(is_detected_exact_v<with_cp, copy_assign_t, with_ex_cp<with_cp> >);
        static_assert(is_detected_exact_v<with_cp*, copy_assign_t, with_ex_cp<with_cp*> >);
        static_assert(is_detected_exact_v<dummy, std::type_identity_t, dummy>);
        static_assert(is_detected_exact_v<nonesuch, std::type_identity_t, nonesuch>);
        static_assert(!is_detected_exact_v<dummy, decl_difference_t, dummy>);
        static_assert(!is_detected_exact_v<nonesuch, decl_difference_t, nonesuch>);
        static_assert(!is_detected_exact_v<std::type_identity<nonesuch>, decl_difference_t, nonesuch>);
        static_assert(!is_detected_exact_v<std::type_identity<nonesuch>, decl_difference_t, std::type_identity<nonesuch> >);
        static_assert(is_detected_exact_deprecated_v<nonesuch, decl_difference_t, nonesuch>);

        static_assert(std::is_same_v<int16_t, difference_t<with_decl_difference<int16_t>>>);
        static_assert(std::is_same_v<int32_t, difference_t<with_decl_difference<int32_t>>>);
        static_assert(std::is_same_v<int64_t, difference_t<with_decl_difference<int64_t>>>);
        static_assert(std::is_same_v<ptrdiff_t, difference_t<with_decl_difference<ptrdiff_t>>>);
        static_assert(std::is_same_v<ptrdiff_t, difference_t<without_decl_difference>>);

        D_ASSERT(!errno);
    }

    void test_conditional_op() noexcept
    {
        static_assert(conditional_op_t<false, std::conjunction, std::true_type, std::false_type>::value);
        static_assert(!conditional_op_t<false, std::conjunction, std::false_type, std::false_type>::value);
        static_assert(!conditional_op_t<true, std::conjunction, std::true_type, std::false_type>::value);
        static_assert(conditional_op_t<true, std::conjunction, std::true_type, std::true_type>::value);
        static_assert(conditional_op_t<true, std::disjunction, std::true_type, std::false_type>::value);
        static_assert(!conditional_op_t<true, std::disjunction, std::false_type, std::false_type>::value);

        static_assert(!conditional_op_or_t<true, std::false_type, std::conjunction, std::true_type, std::false_type>::value);
        static_assert(conditional_op_or_t<true, std::false_type, std::conjunction, std::true_type, std::true_type>::value);
        static_assert(conditional_op_or_t<true, std::false_type, std::disjunction, std::true_type, std::false_type>::value);
        static_assert(!conditional_op_or_t<true, std::false_type, std::disjunction, std::false_type, std::false_type>::value);

        static_assert(!conditional_op_or_t<false, std::false_type, std::conjunction, std::true_type, std::false_type>::value);
        static_assert(!conditional_op_or_t<false, std::false_type, std::conjunction, std::true_type, std::true_type>::value);
        static_assert(!conditional_op_or_t<false, std::false_type, std::disjunction, std::true_type, std::false_type>::value);
        static_assert(!conditional_op_or_t<false, std::false_type, std::disjunction, std::false_type, std::false_type>::value);

        static_assert(conditional_op_or_t<false, std::true_type, std::conjunction, std::true_type, std::false_type>::value);
        static_assert(conditional_op_or_t<false, std::true_type, std::conjunction, std::true_type, std::true_type>::value);
        static_assert(conditional_op_or_t<false, std::true_type, std::disjunction, std::true_type, std::false_type>::value);
        static_assert(conditional_op_or_t<false, std::true_type, std::disjunction, std::false_type, std::false_type>::value);

        D_ASSERT(!errno);
    }

    template<class T>
    void test_conditional_add_const0() noexcept
    {
        static_assert(std::is_same_v<conditional_add_const_t<true, T>, std::add_const_t<T>>);
        static_assert(std::is_same_v<conditional_add_const_t<false, T>, T>);
        D_ASSERT(!errno);
    }

    template<class T>
    void test_conditional_add_const() noexcept
    {
        test_conditional_add_const0<T>();
        test_conditional_add_const0<std::add_const_t<T>>();
    }

    template<class T>
    void test_conditional_add_pointer0() noexcept
    {
        static_assert(std::is_same_v<conditional_add_pointer_t<true, T>, std::add_pointer_t<T>>);
        static_assert(std::is_same_v<conditional_add_pointer_t<false, T>, T>);
        D_ASSERT(!errno);
    }

    template<class T>
    void test_conditional_add_pointer() noexcept
    {
        test_conditional_add_pointer0<T>();
        test_conditional_add_pointer0<std::add_pointer_t<T>>();
    }

    void test_conditional_add_const_all() noexcept
    {
        test_conditional_add_const<int>();
        test_conditional_add_const<std::byte>();
        test_conditional_add_const<void>();
    }

    void test_conditional_add_pointer_all() noexcept
    {
        test_conditional_add_pointer<int>();
        test_conditional_add_pointer<std::byte>();
        test_conditional_add_pointer<void>();
    }

    void test_copy_const() noexcept
    {
        static_assert(std::is_same_v<copy_const_t<const int, int>, const int>);
        static_assert(std::is_same_v<copy_const_t<int, int>, int>);
        static_assert(std::is_same_v<copy_const_t<const int, char>, const char>);
        static_assert(std::is_same_v<copy_const_t<int, char>, char>);
        static_assert(std::is_same_v<copy_const_t<const int, unsigned>, const unsigned>);
        static_assert(std::is_same_v<copy_const_t<int, unsigned>, unsigned>);
        D_ASSERT(!errno);
    }

    void test_copy_pointer() noexcept
    {
        static_assert(std::is_same_v<copy_pointer_t<int*, int>, int*>);
        static_assert(std::is_same_v<copy_pointer_t<int, int>, int>);
        static_assert(std::is_same_v<copy_pointer_t<int*, int*>, int**>);
        static_assert(std::is_same_v<copy_pointer_t<int, int*>, int*>);
        static_assert(std::is_same_v<copy_pointer_t<int*, char>, char*>);
        static_assert(std::is_same_v<copy_pointer_t<int, char>, char>);
        static_assert(std::is_same_v<copy_pointer_t<int*, char*>, char**>);
        static_assert(std::is_same_v<copy_pointer_t<int, char*>, char*>);
        static_assert(std::is_same_v<copy_pointer_t<int*, unsigned>, unsigned*>);
        static_assert(std::is_same_v<copy_pointer_t<int, unsigned>, unsigned>);
        D_ASSERT(!errno);
    }

    void test_replace_type() noexcept
    {
        static_assert(std::is_same_v<replace_t<int, unsigned, char>, int>);
        static_assert(std::is_same_v<replace_t<unsigned, unsigned, char>, char>);
        static_assert(std::is_same_v<replace_t<char, unsigned, char>, char>);
        D_ASSERT(!errno);
    }

    void test_remove_enum() noexcept
    {
        enum class u8_enum : uint8_t
        {};

        enum class i16_enum : int16_t
        {};

        static_assert(std::is_same_v<remove_enum_t<double>, double>);
        static_assert(std::is_same_v<remove_enum_t<int>, int>);
        static_assert(std::is_same_v<remove_enum_t<i16_enum>, int16_t>);
        static_assert(std::is_same_v<remove_enum_t<u8_enum>, uint8_t>);
        D_ASSERT(!errno);
    }

    void test_unsigned_or() noexcept
    {
        static_assert(std::is_same_v<unsigned_or_t<float>, float>);
        static_assert(std::is_same_v<unsigned_or_t<int>, unsigned>);
        static_assert(std::is_same_v<unsigned_or_t<unsigned>, unsigned>);
        D_ASSERT(!errno);
    }

    void test_add_const_pointer() noexcept
    {
        static_assert(std::is_same_v<add_const_pointer_t<void*>, const void*>);
        static_assert(std::is_same_v<add_const_pointer_t<const void*>, const void*>);
        static_assert(std::is_same_v<add_const_pointer_t<void* const>, const void* const>);
        static_assert(std::is_same_v<add_const_pointer_t<const void* const>, const void* const>);
        static_assert(std::is_same_v<add_const_pointer_t<void**>, void* const*>);
        static_assert(std::is_same_v<add_const_pointer_t<void* const*>, void* const*>);
        static_assert(std::is_same_v<add_const_pointer_t<void** const>, void* const* const>);
        static_assert(std::is_same_v<add_const_pointer_t<void* const* const>, void* const* const>);
        static_assert(std::is_same_v<add_const_pointer_t<const void**>, const void* const*>);
        static_assert(std::is_same_v<add_const_pointer_t<const void** const>, const void* const* const>);
        static_assert(std::is_same_v<add_const_pointer_t<const void* const* const>, const void* const* const>);
        D_ASSERT(!errno);
    }

    void test_is_const_convertible() noexcept
    {
        static_assert(is_const_convertible_v<void, void>);
        static_assert(is_const_convertible_v<int, int>);
        static_assert(is_const_convertible_v<int, const int>);
        static_assert(is_const_convertible_v<const int, const int>);
        static_assert(!is_const_convertible_v<const int, int>);
        D_ASSERT(!errno);
    }

    void test_is_sameuncvref() noexcept
    {
        static_assert(is_same_uncvref_v<const int&, int>);
        static_assert(is_same_uncvref_v<int, const int&>);
        static_assert(!is_same_uncvref_v<int*, const int&>);
        static_assert(!is_same_uncvref_v<float, const int&>);
        static_assert(!is_same_uncvref_v<int, float>);
        D_ASSERT(!errno);
    }

    void test_has_assignment_op() noexcept
    {
        static_assert(has_assignment_op_v<int, int>);
        static_assert(has_assignment_op_v<int&, int>);
        static_assert(has_assignment_op_v<int&, const int>);
        static_assert(has_assignment_op_v<int, int&>);
        static_assert(has_assignment_op_v<int&, int&>);
        static_assert(has_assignment_op_v<int, short>);
        static_assert(has_assignment_op_v<double, int>);
        static_assert(has_assignment_op_v<std::span<int>, std::vector<int>&>);
        static_assert(!has_assignment_op_v<std::span<int>, std::vector<int>&&>);
        D_ASSERT(!errno);
    }

    namespace private_detail_test_call_is_detected
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

    void test_call_is_detected() noexcept
    {
        using namespace private_detail_test_call_is_detected;

        static_assert(call_is_detected_v<functor_for<0>, arg<0>>);
        static_assert(!call_is_detected_v<functor_for<0>, arg<1>>);
        static_assert(call_is_detected_v<functor_for<0, 1>, arg<0>, arg<1>>);
        static_assert(!call_is_detected_v<functor_for<0, 1>, arg<0>, arg<0>>);
        static_assert(!call_is_detected_v<functor_for<0, 1>, arg<1>, arg<0>>);
        static_assert(!call_is_detected_v<functor_for<0, 1>, arg<1>, arg<1>>);
        D_ASSERT(!errno);
    }

    void test_u_prev_next() noexcept
    {
        static_assert(1.0 == u_next(0.0));
        static_assert(-1.0 == u_prev(0.0));
        static_assert(1 == u_next(0));
        static_assert(-1 == u_prev(0));
        D_ASSERT(!errno);
    }
}

void test_type_traits() noexcept
{
    test_ordered_overload();
    test_call_if_exist();
    test_member_detector();
    test_conditional_op();
    test_conditional_add_const_all();
    test_conditional_add_pointer_all();
    test_copy_const();
    test_copy_pointer();
    test_replace_type();
    test_remove_enum();
    test_unsigned_or();
    test_add_const_pointer();
    test_is_const_convertible();
    test_is_sameuncvref();
    test_has_assignment_op();
    test_call_is_detected();
    test_u_prev_next();
}