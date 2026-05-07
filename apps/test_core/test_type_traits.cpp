#include <core/type_traits.h>

#include <vector>
#include <span>
#include <array>


namespace
{
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

        static_assert(is_detected_v<copy_assign_t, with_cp>);
        static_assert(!is_detected_v<copy_assign_t, without_cp>);

        static_assert(std::is_same_v<dummy, detected_t<decl_difference_t, dummy> >);
        static_assert(std::is_same_v<dummy, detected_t<decl_difference_t, ttypes<dummy> > >);
        static_assert(std::is_same_v<ptrdiff_t, detected_t<decl_difference_t, with_decl_difference<ptrdiff_t>> >);
        
        static_assert(is_detected_exact_v<with_cp&, copy_assign_t, with_cp>);
        static_assert(is_detected_exact_v<void, copy_assign_t, with_void_cp>);
        static_assert(is_detected_exact_v<with_cp, copy_assign_t, with_ex_cp<with_cp> >);
        static_assert(is_detected_exact_v<with_cp*, copy_assign_t, with_ex_cp<with_cp*> >);
        static_assert(is_detected_exact_v<dummy, std::type_identity_t, dummy>);
        static_assert(is_detected_exact_v<ttypes<dummy>, std::type_identity_t, ttypes<dummy>>);
        static_assert(!is_detected_exact_v<dummy, decl_difference_t, dummy>);
        static_assert(!is_detected_exact_v<ttypes<dummy>, decl_difference_t, ttypes<dummy>>);
        static_assert(is_detected_exact_deprecated_v<dummy, decl_difference_t, dummy>);

        static_assert(std::is_same_v<int16_t, difference_t<with_decl_difference<int16_t>>>);
        static_assert(std::is_same_v<int32_t, difference_t<with_decl_difference<int32_t>>>);
        static_assert(std::is_same_v<int64_t, difference_t<with_decl_difference<int64_t>>>);
        static_assert(std::is_same_v<ptrdiff_t, difference_t<with_decl_difference<ptrdiff_t>>>);
        static_assert(std::is_same_v<ptrdiff_t, difference_t<without_decl_difference>>);

        D_ASSERT(!errno);
    }

    void test_is_same_or() noexcept
    {
        static_assert(is_same_or_v<char, int, char>);
        static_assert(!is_same_or_v<const char, int, char>);
        static_assert(is_same_or_v<int, int, char>);
        static_assert(!is_same_or_v<const int, int, char>);
        static_assert(is_same_or_v<const int, int, char, const int>);

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

    void test_copy_signed() noexcept
    {
        static_assert(std::is_same_v<copy_signed_t<int, int>, int>);
        static_assert(std::is_same_v<copy_signed_t<int, const int>, const int>);
        static_assert(std::is_same_v<copy_signed_t<const int, const int>, const int>);
        static_assert(std::is_same_v<copy_signed_t<const int, int>, int>);

        static_assert(std::is_same_v<copy_signed_t<unsigned int, int>, unsigned int>);
        static_assert(std::is_same_v<copy_signed_t<unsigned int, const int>, const unsigned int>);
        static_assert(std::is_same_v<copy_signed_t<const unsigned int, const int>, const unsigned int>);
        static_assert(std::is_same_v<copy_signed_t<const unsigned int, int>, unsigned int>);

        static_assert(std::is_same_v<copy_signed_t<unsigned int, unsigned int>, unsigned int>);
        static_assert(std::is_same_v<copy_signed_t<unsigned int, const unsigned int>, const unsigned int>);
        static_assert(std::is_same_v<copy_signed_t<const unsigned int, const unsigned int>, const unsigned int>);
        static_assert(std::is_same_v<copy_signed_t<const unsigned int, unsigned int>, unsigned int>);

        static_assert(std::is_same_v<copy_signed_t<int, unsigned int>, int>);
        static_assert(std::is_same_v<copy_signed_t<int, const unsigned int>, const int>);
        static_assert(std::is_same_v<copy_signed_t<const int, const unsigned int>, const int>);
        static_assert(std::is_same_v<copy_signed_t<const int, unsigned int>, int>);

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
        static_assert(!is_nonbool_integral_v<float>);
        static_assert(is_nonbool_integral_v<int>);
        static_assert(!is_nonbool_integral_v<bool>);
        static_assert(std::is_integral_v<bool>);
        static_assert(std::is_same_v<unsigned_or_t<float>, float>);
        static_assert(std::is_same_v<unsigned_or_t<int>, unsigned>);
        static_assert(std::is_same_v<unsigned_or_t<unsigned>, unsigned>);
        static_assert(std::is_same_v<unsigned_or_t<bool>, bool>);
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

    void test_has_no_unique_address() noexcept
    {
        static_assert(has_no_unique_address_v<dummy>);
        static_assert(has_no_unique_address_v<ttypes<char, long, int>>);
        static_assert(has_no_unique_address_v<std::true_type>);
        static_assert(!has_no_unique_address_v<bool>);
        static_assert(!has_no_unique_address_v<char>);
        static_assert(!has_no_unique_address_v<void*>);
        static_assert(!has_no_unique_address_v<int>);
        D_ASSERT(!errno);
    }

    void test_is_address() noexcept
    {
        struct A
        {
            int m;
            void f() {}
        };

        int A::* mem_data_ptr = &A::m;
        void (A::* mem_fun_ptr)() = &A::f;

        static_assert(is_address_v<int*>);
        static_assert(is_address_v<int*&>);
        static_assert(is_address_v<int*&&>);
        static_assert(is_address_v<const int*>);
        static_assert(is_address_v<const int* &>);
        static_assert(is_address_v<const int* &&>);
        static_assert(is_address_v<const int* const>);
        static_assert(is_address_v<const int* const>);
        static_assert(is_address_v<const int* const &>);
        static_assert(is_address_v<const int* const &&>);
        static_assert(is_address_v<void ()>);
        static_assert(is_address_v<void (*)>);
        static_assert(is_address_v<void (*const)>);
        static_assert(is_address_v<decltype(mem_data_ptr)>);
        static_assert(is_address_v<decltype(mem_fun_ptr)>);
        static_assert(is_address_v<std::nullptr_t>);
        static_assert(!is_address_v<A>);
        static_assert(is_address_v<A*>);
        static_assert(is_address_v<void*>);
        static_assert(is_address_v<const void*>);
    }

    void test_has_qualifier() noexcept
    {
        enum class test_enum
        {};

        struct test_struct {};

        static_assert(!has_qualifier_v<int>);
        static_assert(!has_qualifier_v<test_enum>);
        static_assert(!has_qualifier_v<test_struct>);
        static_assert(has_qualifier_v<const int>);
        static_assert(has_qualifier_v<int&>);
        static_assert(has_qualifier_v<int&&>);
        static_assert(has_qualifier_v<int*>);
        static_assert(has_qualifier_v<int[]>);
        static_assert(has_qualifier_v<int[3]>);
        static_assert(has_qualifier_v<std::nullptr_t>);
        static_assert(has_qualifier_v<const int&>);
        static_assert(has_qualifier_v<const int&&>);
        static_assert(has_qualifier_v<const int*>);
        static_assert(has_qualifier_v<int*&>);
        static_assert(has_qualifier_v<int**>);
        D_ASSERT(!errno);
    }

    void test_is_unqualified_class() noexcept
    {
        enum class test_enum
        {};

        struct test_struct {};

        static_assert(!is_unqualified_class_v<int>);
        static_assert(!is_unqualified_class_v<nullptr_t>); static_assert(!std::is_class_v<nullptr_t>);
        static_assert(!is_unqualified_class_v<test_enum>); static_assert(!std::is_class_v<test_enum>);

        static_assert(!is_unqualified_class_v<const test_struct>); static_assert(std::is_class_v<const test_struct>);
        static_assert(!is_unqualified_class_v<test_struct&>); static_assert(!std::is_class_v<test_struct&>);
        static_assert(!is_unqualified_class_v<test_struct&&>); static_assert(!std::is_class_v<test_struct&&>);
        static_assert(!is_unqualified_class_v<test_struct*>); static_assert(!std::is_class_v<test_struct*>);
        static_assert(!is_unqualified_class_v<test_struct[]>); static_assert(!std::is_class_v<test_struct[]>);
        static_assert(!is_unqualified_class_v<test_struct[3]>); static_assert(!std::is_class_v<test_struct[3]>);

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

    void test_is_same_size() noexcept
    {
        static_assert(is_same_size<2u, 2u>::value);
        static_assert(!is_same_size<2u, 3u>::value);
        static_assert(!is_same_size<3u, 2u>::value);
        static_assert(is_same_size<3u, 3u>::value);
        static_assert(!std::negation<is_same_size<2u, 2u>>::value);
        static_assert(std::negation<is_same_size<2u, 3u>>::value);
        static_assert(std::negation<is_same_size<3u, 2u>>::value);
        static_assert(!std::negation<is_same_size<3u, 3u>>::value);
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

    void test_u_prev_next() noexcept
    {
        static_assert(1.0 == u_next(0.0));
        static_assert(-1.0 == u_prev(0.0));
        static_assert(1 == u_next(0));
        static_assert(-1 == u_prev(0));
        D_ASSERT(!errno);
    }

    namespace private_detail_test_noexcept
    {
        void with_except()
        {
            delete new char;
        }
    }

    void test_remove_noexcept() noexcept
    {
        using namespace private_detail_test_noexcept;
        [[maybe_unused]] constexpr auto use_with_except = with_except;

        using fn_noexept_t = std::decay_t<decltype(test_remove_noexcept)>;
        using fn_exept_t = std::decay_t<decltype(with_except)>;

        static_assert(!std::is_same_v<fn_noexept_t, fn_exept_t>);
        static_assert(std::is_same_v<remove_noexcept_t<fn_noexept_t>, fn_exept_t>);
        static_assert(std::is_same_v<remove_noexcept_t<fn_exept_t>, fn_exept_t>);
        D_ASSERT(!errno);
    }

    void test_add_noexcept() noexcept
    {
        using namespace private_detail_test_noexcept;
        [[maybe_unused]] constexpr auto use_with_except = with_except;

        using fn_noexept_t = std::decay_t<decltype(test_remove_noexcept)>;
        using fn_exept_t = std::decay_t<decltype(with_except)>;

        static_assert(!std::is_same_v<fn_noexept_t, fn_exept_t>);
        static_assert(std::is_same_v<add_noexcept_t<fn_exept_t>, fn_noexept_t>);
        static_assert(std::is_same_v<add_noexcept_t<fn_noexept_t>, fn_noexept_t>);
        D_ASSERT(!errno);
    }

    void test_brace_constructible() noexcept
    {
        static_assert(is_brace_constructible_v<int, int>);
        static_assert(!is_brace_constructible_v<int, double>);

        struct test_brace_t
        {
            int i;
            double d;
            const char* s;
        };

        static_assert(is_brace_constructible_v<test_brace_t, int>);
        static_assert(is_brace_constructible_v<test_brace_t, int, double>);
        static_assert(is_brace_constructible_v<test_brace_t, int, double, const char*>);
        static_assert(!is_brace_constructible_v<test_brace_t, const char*>);

        using iarr2_t = std::array<int, 2u>;
        static_assert(is_brace_constructible_v<iarr2_t, int>);
        static_assert(is_brace_constructible_v<iarr2_t, int, int>);
        static_assert(!is_brace_constructible_v<iarr2_t, int, int, int>);
        D_ASSERT(!errno);
    }
}

void test_type_traits() noexcept
{
    test_member_detector();
    test_is_same_or();
    test_conditional_op();
    test_conditional_add_const_all();
    test_conditional_add_pointer_all();
    test_copy_const();
    test_copy_pointer();
    test_copy_signed();
    test_replace_type();
    test_remove_enum();
    test_unsigned_or();
    test_add_const_pointer();
    test_has_no_unique_address();
    test_is_address();
    test_has_qualifier();
    test_is_unqualified_class();
    test_is_const_convertible();
    test_is_same_size();
    test_is_sameuncvref();
    test_has_assignment_op();
    test_u_prev_next();
    test_remove_noexcept();
    test_add_noexcept();
    test_brace_constructible();
}