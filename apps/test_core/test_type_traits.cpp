#include <core/type_traits.h>

#include <cerrno>
#include <cinttypes>

#include <core/assert.h>

namespace
{
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
        static_assert(std::is_same_v<add_unsigned_t<float>, float>);
        static_assert(std::is_same_v<add_unsigned_t<int>, unsigned>);
        static_assert(std::is_same_v<add_unsigned_t<unsigned>, unsigned>);
    }

    D_ASSERT(!errno);
}