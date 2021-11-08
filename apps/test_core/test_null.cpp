#include <chrono>

#include <core/assert.h>
#include <core/null.h>
#include <core/resouce.h>

struct skip_op
{
    template<class T>
    void operator () (T&&, resource_destroy_t) const noexcept
    {}
};

void test_null() noexcept
{
    struct test_null
    {
        void* ptr;
        int i;
        bool b;
    };

    static_assert( std::is_same_v<null_t<void*>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<void*const>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<const void*>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<const void*const>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<int*>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<int*const>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<const int*>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<const int*const>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<test_null*>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<test_null*const>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<const test_null*>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<const test_null*const>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<std::nullptr_t>, std::nullptr_t> );
    static_assert( std::is_same_v<null_t<const std::nullptr_t>, std::nullptr_t> );
    
    using nullint_t = null_t<int>;
    static_assert( !std::is_same_v<nullint_t, std::nullptr_t> );
    static_assert( std::is_same_v<nullint_t, null_t<const int>> );
    static_assert( std::is_same_v<null_t<nullint_t>, nullint_t> );
    static_assert( std::is_same_v<null_t<const nullint_t>, nullint_t> );

    using nulltest_t = null_t<test_null>;
    static_assert( !std::is_same_v<nulltest_t, std::nullptr_t> );
    static_assert( !std::is_same_v<nulltest_t, null_t<int>> );
    static_assert( std::is_same_v<nulltest_t, null_t<const test_null>> );
    static_assert( std::is_same_v<null_t<nulltest_t>, nulltest_t> );
    static_assert( std::is_same_v<null_t<const nulltest_t>, nulltest_t> );

    test_null null = nulltest_t{};
    D_ASSERT(!null.b);
    D_ASSERT(!null.i);
    D_ASSERT(!null.ptr);

    unique_resource<test_null, skip_op> default_unique;

    D_ASSERT(!default_unique->b);
    D_ASSERT(!default_unique->i);
    D_ASSERT(!default_unique->ptr);

    shared_resource<test_null, skip_op> default_linked;
    D_ASSERT(!default_linked->b);
    D_ASSERT(!default_linked->i);
    D_ASSERT(!default_linked->ptr);

    struct test_time_null
    {
        std::chrono::milliseconds ms;
    };

    test_time_null tm_null = null_t<test_time_null>{};
    D_ASSERT(!tm_null.ms.count());

    unique_resource<test_time_null, skip_op> default_unique_tm;
    D_ASSERT(!default_unique_tm->ms.count());

    shared_resource<test_time_null, skip_op> default_linked_tm;
    D_ASSERT(!default_linked_tm->ms.count());
}