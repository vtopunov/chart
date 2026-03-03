#include <core/fwd.h>

#include <type_traits>


namespace
{
    void test_delete_default_ca() noexcept
    {
        {
            struct delete_default_ca
            {
                D_DISABLE_COPY_CA(delete_default_ca);
            };

            static_assert(!std::is_default_constructible_v<delete_default_ca>);
            static_assert(!std::is_copy_constructible_v<delete_default_ca>);
            static_assert(!std::is_move_constructible_v<delete_default_ca>);
            static_assert(!std::is_copy_assignable_v<delete_default_ca>);
            static_assert(!std::is_move_assignable_v<delete_default_ca>);
        }

        D_ASSERT(!errno);
    }

    namespace private_detail_test_no_overload
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

    void test_no_overload() noexcept
    {
        using namespace private_detail_test_no_overload;

        constexpr with_method swt{};
        constexpr without_method swot{};

        static_assert(call_method_if_exist(swt));
        static_assert(!call_method_if_exist(swot));

        D_ASSERT(!errno);
    }

}


void test_fwd() noexcept
{
    test_delete_default_ca();
    test_no_overload();
}