#include <core/utility.h>
#include <core/assert.h>

void test_utility() noexcept
{
    {
        static_assert( std::is_same_v<add_immutable_t<int>, const int> );
        static_assert( std::is_same_v<add_immutable_t<const int>, const int> );
        static_assert( std::is_same_v<add_immutable_t<int*>, const int*const> );
        static_assert( std::is_same_v<add_immutable_t<const int*>, const int*const> );
        static_assert( std::is_same_v<add_immutable_t<int*const>, const int*const> );
        static_assert( std::is_same_v<add_immutable_t<const int*const>, const int*const> );
        static_assert( std::is_same_v<add_immutable_t<int&>, const int&> );
        static_assert( std::is_same_v<add_immutable_t<const int&>, const int&> );
        static_assert( std::is_same_v<add_immutable_t<int*&>, const int*const&> );
        static_assert( std::is_same_v<add_immutable_t<int*const&>, const int*const&> );
        static_assert( std::is_same_v<add_immutable_t<const int*&>, const int*const&> );
        static_assert( std::is_same_v<add_immutable_t<const int*const&>, const int*const&> );
    }

    {
        int value = 0;
        int& value_ref = value;
        int* value_ptr = &value;
        auto value_cptr = as_immutable(value_ptr);

        static_assert( std::is_same_v<decltype( as_immutable(value) ), const int&> );
        static_assert( std::is_same_v<decltype( as_immutable(value_ref) ), const int&> );
        static_assert( std::is_same_v<decltype( as_immutable(value_ptr) ), const int*> );
        static_assert( std::is_same_v<decltype( value_cptr ), const int*> );
    }

    D_ASSERT(!errno);
}