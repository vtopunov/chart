#include <core/fwd.h>

#include <type_traits>


void test_fwd() noexcept
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