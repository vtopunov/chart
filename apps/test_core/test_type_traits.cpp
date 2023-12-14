#include <core/type_traits.h>

#include <cerrno>
#include <cinttypes>

#include <core/assert.h>


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
    }

    void test_member_detector() noexcept
    {
        using namespace private_detail_test_member_detector;

        static_assert(is_detected_v<copy_assign_t, with_cp>);
        static_assert(!is_detected_v<copy_assign_t, without_cp>);

        static_assert(is_detected_exact_v<with_cp&, copy_assign_t, with_cp>);
        static_assert(is_detected_exact_v<void, copy_assign_t, with_void_cp>);
        static_assert(is_detected_exact_v<with_cp, copy_assign_t, with_ex_cp<with_cp> >);
        static_assert(is_detected_exact_v<with_cp*, copy_assign_t, with_ex_cp<with_cp*> >);

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
        D_ASSERT(!errno);
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
        D_ASSERT(!errno);
    }


}

void test_type_traits() noexcept
{
    test_member_detector();
    test_conditional_op();

    {
        test_conditional_add_const<int>();
        test_conditional_add_const<std::byte>();
        test_conditional_add_const<void>();
    }

    {
        test_conditional_add_pointer<int>();
        test_conditional_add_pointer<std::byte>();
        test_conditional_add_pointer<void>();
    }

    {
        static_assert(std::is_same_v<copy_const_t<const int, int>, const int>);
        static_assert(std::is_same_v<copy_const_t<int, int>, int>);
        static_assert(std::is_same_v<copy_const_t<const int, char>, const char>);
        static_assert(std::is_same_v<copy_const_t<int, char>, char>);
        static_assert(std::is_same_v<copy_const_t<const int, unsigned>, const unsigned>);
        static_assert(std::is_same_v<copy_const_t<int, unsigned>, unsigned>);
    }

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
    }

    {
        static_assert(std::is_same_v<replace_t<int, unsigned, char>, int>);
        static_assert(std::is_same_v<replace_t<unsigned, unsigned, char>, char>);
        static_assert(std::is_same_v<replace_t<char, unsigned, char>, char>);
    }

    {
        enum class u8_enum : uint8_t
        {};

        enum class i16_enum : int16_t
        {};

        static_assert(std::is_same_v<remove_enum_t<double>, double>);
        static_assert(std::is_same_v<remove_enum_t<int>, int>);
        static_assert(std::is_same_v<remove_enum_t<i16_enum>, int16_t>);
        static_assert(std::is_same_v<remove_enum_t<u8_enum>, uint8_t>);
    }

    {
        static_assert(std::is_same_v<unsigned_or_t<float>, float>);
        static_assert(std::is_same_v<unsigned_or_t<int>, unsigned>);
        static_assert(std::is_same_v<unsigned_or_t<unsigned>, unsigned>);
    }

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
    }

    D_ASSERT(!errno);
}