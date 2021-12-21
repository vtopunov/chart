#include <functional>
#include <chrono>

#include <core/resouce.h>
#include <core/assert.h>

namespace
{
    struct skip_op
    {
        template<class T>
        void operator () (T&&) const noexcept
        {}
    };

    template<class resource_type, class D>
    struct unsafe_resource
    {
        resource_type* operator -> () noexcept
        {
            return &h;
        }

        resource_type h;
        intrusive_list_node c;
    };

#pragma warning( push )
#pragma warning( disable : 26418 ) 
    template<class T, class D>
    unsafe_resource<T, D>& unsafe(const shared_resource<T, D>& safe) noexcept
    {
        using safe_t = shared_resource<T, D>;
        using unsafe_t = unsafe_resource<T, D>;

        static_assert(sizeof(safe_t) == sizeof(unsafe_t));
        static_assert(alignof(safe_t) == alignof(unsafe_t));
        return (unsafe_t&) safe;
    }
#pragma warning(pop)

    template<class T, class D>
    const intrusive_list_node* node(const shared_resource<T, D>& safe) noexcept
    {
        return &unsafe(safe).c;
    }

    template<class T, class D>
    const intrusive_list_node* prev(const shared_resource<T, D>& safe) noexcept
    {
        return node(safe)->prev;
    }

    template<class T, class D>
    const intrusive_list_node* next(const shared_resource<T, D>& safe) noexcept
    {
        return node(safe)->next;
    }

    struct tested_resouce
    {
        tested_resouce() noexcept = default;

        tested_resouce(int right) noexcept
            : value{ right }
        {}

        constexpr explicit operator bool() const noexcept
        {
            return value;
        }

        int value = 0;
        std::function<void(const tested_resouce&)> check_close;
    };

    struct tested_resouce_deleter
    {
        void operator () (const tested_resouce& resouce) const noexcept
        {
            if (resouce.check_close)
            {
                resouce.check_close(resouce);
            }
        }
    };

    using tested_unique = unique_resource<tested_resouce, tested_resouce_deleter>;
    using tested_linked = shared_resource<tested_resouce, tested_resouce_deleter>;

    static_assert(std::is_same_v<null_t<tested_unique>, null_t<tested_resouce>>);
    static_assert(std::is_same_v<null_t<tested_linked>, null_t<tested_resouce>>);

    struct verifiable_resource
    {
        mutable bool is_valid;

        constexpr explicit operator bool() const noexcept
        {
            return is_valid;
        }
    };

    using verifiable_unique = unique_resource<verifiable_resource, skip_op>;
    using verifiable_linked = shared_resource<verifiable_resource, skip_op>;

    static_assert(!std::is_same_v<null_t<tested_resouce>, null_t<verifiable_resource>>);
    static_assert(std::is_same_v<null_t<verifiable_unique>, null_t<verifiable_resource>>);
    static_assert(std::is_same_v<null_t<verifiable_linked>, null_t<verifiable_resource>>);
}

void test_resource() noexcept
{
#pragma warning( push )
#pragma warning( disable : 26415 ) 
#pragma warning( disable : 26418 )
    constexpr struct
    {
        bool operator () (const tested_linked& h1, int value) const noexcept
        {
            D_ASSERT(next(h1) == node(h1) && prev(h1) == node(h1));
            D_ASSERT(h1.r().value == value);
            return true;
        }

        bool operator () (const tested_linked& h1, const tested_linked& h2, int value) const noexcept
        {
            D_ASSERT(next(h1) == node(h2) && prev(h1) == node(h2));
            D_ASSERT(next(h2) == node(h1) && prev(h2) == node(h1));
            D_ASSERT(h1.r().value == value && h2.r().value == value);
            return true;
        }

        bool operator () (const tested_linked& h1, const tested_linked& h2, const tested_linked& h3, int value) const noexcept
        {
            D_ASSERT(next(h1) == node(h2) && prev(h1) == node(h3));
            D_ASSERT(next(h2) == node(h3) && prev(h2) == node(h1));
            D_ASSERT(next(h3) == node(h1) && prev(h3) == node(h2));
            D_ASSERT(h1.r().value == value && h2.r().value == value && h3.r().value == value);
            return true;
        };
    } check;
#pragma warning(pop)

    {
        {
            verifiable_unique safe;
            D_ASSERT(!safe.r().is_valid);
            D_ASSERT(!safe);
            safe.r().is_valid = true;
            D_ASSERT(safe);
            safe.r().is_valid = false;
            D_ASSERT(!safe);
        }

        {
            verifiable_linked safe;
            D_ASSERT(!safe.r().is_valid);
            D_ASSERT(!safe);
            safe.r().is_valid = true;
            D_ASSERT(safe);
            safe.r().is_valid = false;
            D_ASSERT(!safe);
        }
    }

    tested_linked h1{ resource_construct, 1 };
    check(h1, 1);

    { // self assignment
        h1 = h1;
        check(h1, 1);
    }

    {   // smart resouce closure
        int closed_value = 0;
        {
            tested_linked h2{ resource_construct, 2 };
            unsafe(h2).h.check_close = [&closed_value] (const tested_resouce& closing_handle) noexcept
            {
                D_ASSERT(closed_value != 2 && closing_handle.value == 2);
                closed_value = closing_handle.value;
            };
        }
        D_ASSERT(closed_value == 2);
    }

    { // copy constructor
        tested_linked h2{ h1 };
        check(h1, h2, 1);
    }
    check(h1, 1);

    { // assignment initialization 
        int h2_closed_value = 0;
        tested_linked h2{ resource_construct, 2 };
        unsafe(h2).h.check_close = [&h2_closed_value] (const tested_resouce& closing_handle) noexcept
        {
            D_ASSERT(h2_closed_value != 2 && closing_handle.value == 2);
            h2_closed_value = closing_handle.value;
        };
        h2 = h1;
        check(h1, h2, 1);
        D_ASSERT(h2_closed_value == 2);
    }
    check(h1, 1);

    {   // assignment
        tested_linked h2{ resource_construct, 2 };

        int h1_closed_value = 0;
        unsafe(h1).h.check_close = [&h1_closed_value] (const tested_resouce& closing_handle) noexcept
        {
            D_ASSERT(h1_closed_value != 1 && closing_handle.value == 1);
            h1_closed_value = closing_handle.value;
        };

        D_ASSERT(h1.r().value == 1);
        h1 = h2;
        check(h1, h2, 2);
        D_ASSERT(h1_closed_value == 1);
    }
    check(h1, 2);
    unsafe(h1).h.value = 1;

    {   // cyclic assignment
        tested_linked h2{ h1 };
        check(h1, h2, 1);
        h1 = h2;
        check(h1, h2, 1);
        h2 = h1;
        check(h1, h2, 1);
    }
    check(h1, 1);

    {   // cyclic assignment (ref count > 2)
        tested_linked h2{ h1 };
        tested_linked h3{ h2 };
        check(h1, h2, h3, 1);
        h2 = h3;
        check(h3, h2, h1, 1);
        h3 = h2;
        check(h2, h3, h1, 1);
        h1 = h2;
        check(h2, h1, h3, 1);
    }
    check(h1, 1);

    {   // assignment (ref count >= 2)

        int h2_closed_value = 0;
        {
            tested_linked ch1{ h1 };
            check(h1, ch1, 1);

            tested_linked h2{ resource_construct, 2 };
            tested_linked ch2{ h2 };
            check(h2, ch2, 2);

            h2 = h1;
            check(ch1, h1, h2, 1);
            check(ch2, 2);

            ch1 = ch2;
            check(ch1, ch2, 2);
            check(h1, h2, 1);

            h2 = ch2;
            check(ch1, ch2, h2, 2);
            check(h1, 1);

            ch1 = h1;
            check(h2, ch2, 2);
            check(h1, ch1, 1);

            unsafe(h2).h.check_close = [&h2_closed_value] (const tested_resouce& closing_handle) noexcept
            {
                D_ASSERT(h2_closed_value != 2 && closing_handle.value == 2);
                h2_closed_value = closing_handle.value;
            };
        }
        D_ASSERT(h2_closed_value == 2);
    }

    static struct h1_check_dtor_type
    {
        int h1_closed_value = 0;

        void operator () (const tested_resouce& closing_handle) noexcept
        {
            D_ASSERT(h1_closed_value != 1 && closing_handle.value == 1);
            h1_closed_value = closing_handle.value;
        }

        ~h1_check_dtor_type() noexcept
        {
            D_ASSERT(h1_closed_value == 1);
        }
    } h1_check_dtor;

    check(h1, 1);

    unsafe(h1).h.check_close = std::ref(h1_check_dtor);
}
