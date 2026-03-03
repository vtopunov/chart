#include <core/null.h>

#include <chrono>

#include <core/resource.h>


namespace
{
    struct test_struct
    {
        void* ptr;
        int i;
        bool b;
        std::chrono::milliseconds ms;
    };

    struct nullable_with_bool_op
    {
        bool is_valid;

        constexpr explicit operator bool() const noexcept
        {
            return is_valid;
        }
    };

    struct custrom_nullable
    {
        struct null_type
        {
            template<class N>
            constexpr operator N () const noexcept
            {
                return { custrom_nullable{} };
            }
        };

        [[nodiscard]] constexpr bool operator == (const custrom_nullable&) const = default;
        [[nodiscard]] constexpr bool operator != (const custrom_nullable&) const = default;
    };

    struct custrom_nullable2 : custrom_nullable
    {};
}

void test_null() noexcept
{
    static_assert(std::is_trivial_v<test_struct> && std::is_standard_layout_v<test_struct>);
    
    static_assert(std::is_same_v<null_t<void*>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<void* const>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<const void*>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<const void* const>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<int*>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<int* const>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<const int*>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<const int* const>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<test_struct*>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<test_struct* const>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<const test_struct*>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<const test_struct* const>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<std::nullptr_t>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<const std::nullptr_t>, std::nullptr_t>);
    static_assert(std::is_same_v<null_t<std::nullopt_t>, std::nullopt_t>);
    static_assert(std::is_same_v<null_t<const std::nullopt_t>, std::nullopt_t>);
    static_assert(std::is_same_v<null_t<::std::optional<int>>, std::nullopt_t>);
    static_assert(std::is_same_v<null_t<const ::std::optional<int>>, std::nullopt_t>);
    static_assert(std::is_same_v<null_t<nullmem_t>, nullmem_t>);
    static_assert(std::is_same_v<null_t<const nullmem_t>, nullmem_t>);
    static_assert(std::is_same_v<null_t<nullref_t>, nullref_t>);
    static_assert(std::is_constructible_v<std::nullptr_t, decltype(null_v<std::nullptr_t>)>);
    static_assert(std::is_constructible_v<std::nullopt_t, decltype(null_v<std::nullopt_t>)>);
    static_assert(std::is_constructible_v<nullmem_t, decltype(null_v<nullmem_t>)>);
    static_assert(std::is_constructible_v<nullmem_t, decltype(null_v<nulltype_construct_t>)>);
    static_assert(std::is_convertible_v<decltype(null_v<std::nullptr_t>), std::nullptr_t>);
    static_assert(std::is_convertible_v<decltype(null_v<std::nullopt_t>), std::nullopt_t>);
    static_assert(std::is_convertible_v<decltype(null_v<nullmem_t>), nullmem_t>);
    static_assert(std::is_convertible_v<decltype(null_v<nullmem_t>), nullmem_t>);

    {
        using nullint_t = null_t<int>;
        static_assert(!std::is_same_v<nullint_t, std::nullptr_t>);
        static_assert(std::is_same_v<nullint_t, null_t<const int>>);
    }

    {
        using testnull_t = null_t<test_struct>;
        static_assert(!std::is_same_v<testnull_t, std::nullptr_t>);
        static_assert(!std::is_same_v<testnull_t, null_t<int>>);
        static_assert(std::is_same_v<testnull_t, null_t<const test_struct>>);
        static_assert(!is_nullable_v<test_struct>);
    }

    {
        test_struct null = null_t<test_struct>{};
        D_ASSERT(!null.b);
        D_ASSERT(!null.i);
        D_ASSERT(!null.ptr);
    }

    {
        test_struct null = null_v<test_struct>;
        D_ASSERT(!null.b);
        D_ASSERT(!null.i);
        D_ASSERT(!null.ptr);
        D_ASSERT(!null.ms.count());
    }

    {
        using unique_test_struct = unique_resource<test_struct, nothing>;
        unique_test_struct default_unique;
        D_ASSERT(!default_unique.r().b);
        D_ASSERT(!default_unique.r().i);
        D_ASSERT(!default_unique.r().ptr);
        D_ASSERT(!default_unique.r().ms.count());
        static_assert(!is_nullable_v<unique_test_struct>);
    }

    {
        using shared_test_struct = shared_resource<test_struct, nothing>;
        shared_test_struct default_shared;
        D_ASSERT(!default_shared.r().b);
        D_ASSERT(!default_shared.r().i);
        D_ASSERT(!default_shared.r().ptr);
        D_ASSERT(!default_shared.r().ms.count());
        static_assert(!is_nullable_v<shared_test_struct>);
    }

    {
        constexpr auto nullnullable = null_v<nullable_with_bool_op>;
        nullable_with_bool_op var = nullnullable;
        D_ASSERT(!var);
        static_assert(is_nullable_v<nullable_with_bool_op>);
        static_assert(is_nullable_v<unique_resource<nullable_with_bool_op, nothing>>);
        static_assert(is_nullable_v<shared_resource<nullable_with_bool_op, nothing>>);
        D_ASSERT(!has_value(var));
        D_ASSERT(is_null(var));
        D_ASSERT(var == nullnullable);
        D_ASSERT(nullnullable == var);
        D_ASSERT(!(var != nullnullable));
        D_ASSERT(!(nullnullable != var));
        var.is_valid = true;
        D_ASSERT(var);
        D_ASSERT(has_value(var));
        D_ASSERT(!is_null(var));
        D_ASSERT(!(var == nullnullable));
        D_ASSERT(!(nullnullable == var));
        D_ASSERT(var != nullnullable);
        D_ASSERT(nullnullable != var);
    }

    {
        enum class e_null
        {};

        enum class e_invalid
        {
            invalid = 0x43214321
        };

        static_assert(null_v<e_null> == e_null{});
        static_assert(null_v<e_invalid> == e_invalid::invalid);

        static_assert(is_nullable_v<e_null>);
        static_assert(is_nullable_v<e_invalid>);
    }

    {
        constexpr custrom_nullable cn{};
        constexpr custrom_nullable2 cn2{};
        static_assert(cn == null_v<custrom_nullable>);
        static_assert(cn2 == null_v<custrom_nullable>);
    }

    D_ASSERT(!errno);
}