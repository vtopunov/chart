#include <core/reference_wrapper.h>


void test_reference_wrapper() noexcept
{
    {
        constexpr reference_wrapper<int> rw{ nullref };
        static_assert(!rw);
        static_assert(!rw.has_value());

        static_assert(is_nullable_v<reference_wrapper<int>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<int>>);
        static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const int>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<int&>>);
        static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const int&>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<int&&>>);
        static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const int&&>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<reference_wrapper<int>>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<reference_wrapper<int>&>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<const reference_wrapper<int>>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<const reference_wrapper<int>&>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<const reference_wrapper<int>&&>>);
        static_assert(std::is_same_v<const int, remove_reference_wrapper_t<reference_wrapper<const int>>>);
        static_assert(std::is_same_v<const int, remove_reference_wrapper_t<reference_wrapper<const int>&>>);
        static_assert(std::is_same_v<const int, remove_reference_wrapper_t<reference_wrapper<const int>&&>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<const reference_wrapper<int>>>);
        static_assert(std::is_same_v<int, remove_reference_wrapper_t<const reference_wrapper<int>&>>);
        static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const reference_wrapper<const int>>>);
        static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const reference_wrapper<const int>&>>);
    }

    {
        int value = 0;
        reference_wrapper<int> nullrefwv{ nullref };
        reference_wrapper<int> refwv(value);
        reference_wrapper<const int> refwvc(value);
        D_ASSERT(!nullrefwv);
        D_ASSERT(refwv);
        D_ASSERT(refwvc);
        static_assert(std::is_same_v<int&, decltype(::unrefwrap(refwv))>);
        static_assert(std::is_same_v<const int&, decltype(::unrefwrap(refwvc))>);
        D_ASSERT(0 == value);
        refwv.get() = 33;
        D_ASSERT(33 == value);
        D_ASSERT(value == refwvc.get());
        
        {
            int& ref = refwv;
            D_ASSERT(33 == ref);
            ref = 0;
            D_ASSERT(0 == ref);
        }

        const auto set_value = [&value] (int new_value) noexcept
        {
            value = new_value;
        };
        D_ASSERT(0 == value);
        set_value(33);
        D_ASSERT(33 == value);
        set_value(0);
        D_ASSERT(0 == value);

        reference_wrapper<decltype(set_value)> set_value_rw = set_value;
        D_ASSERT(0 == value);
        set_value_rw(33);
        D_ASSERT(33 == value);
        set_value_rw(0);
        D_ASSERT(0 == value);

        const auto set_set_value = [set_value_rw] (int new_value) noexcept
        {
            set_value_rw(new_value);
        };
        D_ASSERT(0 == value);
        set_set_value(33);
        D_ASSERT(33 == value);
        set_set_value(0);
        D_ASSERT(0 == value);

        D_ASSERT(!errno);
    }
}

