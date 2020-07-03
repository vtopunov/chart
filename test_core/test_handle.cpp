#include <functional>

#include <core/handle.h>
#include <core/assert.h>

template<class handle_type>
struct unsafe_handle
{
    handle_type* operator -> () noexcept
    {
        return h;
    }

    handle_type h;
    intrusive_list_node<shared_handle<handle_type>> c;
};

#pragma warning( push )
#pragma warning( disable : 26418 ) 
template<class T>
unsafe_handle<T>& unsafe(const shared_handle<T>& safe) noexcept
{
    static_assert( sizeof(unsafe_handle<T>) == sizeof(shared_handle<T>) );
    static_assert( alignof( unsafe_handle<T> ) == alignof( shared_handle<T> ) );
    return ( unsafe_handle<T>& )safe;
}
#pragma warning(pop)

struct tested_handle
{
public:
    tested_handle() noexcept = default;

    tested_handle(int right) noexcept
        : value{right}
    {}

    explicit constexpr operator bool() const noexcept
    {
        return value;
    }

    int value = 0;
    std::function<void(const tested_handle&)> check_close;
};

void close(const tested_handle& handle) noexcept
{
    if (handle.check_close)
    {
        handle.check_close(handle);
    }
}

struct empty_handle
{};

void close(const empty_handle&) noexcept
{}

void test_handle() noexcept
{
#pragma warning( push )
#pragma warning( disable : 26415 ) 
#pragma warning( disable : 26418 )
    constexpr struct
    {
        bool operator () (const shared_handle<tested_handle>& h1, int value) const noexcept
        {
            D_ASSERT(unsafe(h1).c.next == &h1 && unsafe(h1).c.prev == &h1);
            D_ASSERT(h1->value == value);
            return true;
        }

        bool operator () (const shared_handle<tested_handle>& h1, const shared_handle<tested_handle>& h2, int value) const noexcept
        {
            D_ASSERT(unsafe(h1).c.next == &h2 && unsafe(h1).c.prev == &h2);
            D_ASSERT(unsafe(h2).c.next == &h1 && unsafe(h2).c.prev == &h1);
            D_ASSERT(h1->value == value && h2->value == value);
            return true;
        }

        bool operator () (const shared_handle<tested_handle>& h1, const shared_handle<tested_handle>& h2, const shared_handle<tested_handle>& h3, int value) const noexcept
        {
            D_ASSERT(unsafe(h1).c.next == &h2 && unsafe(h1).c.prev == &h3);
            D_ASSERT(unsafe(h2).c.next == &h3 && unsafe(h2).c.prev == &h1);
            D_ASSERT(unsafe(h3).c.next == &h1 && unsafe(h3).c.prev == &h2);
            D_ASSERT(h1->value == value && h2->value == value && h3->value == value);
            return true;
        };
    } check;
#pragma warning(pop)

    {
        struct handle_with_is_valid : public empty_handle
        {
            mutable bool is_valid = false;

            explicit constexpr operator bool() const noexcept
            {
                return is_valid;
            }
        };

        shared_handle<handle_with_is_valid> safe;

        safe->is_valid = true;
        D_ASSERT(safe);
        safe->is_valid = false;
        D_ASSERT(!safe);
        safe->is_valid = true;
        D_ASSERT(safe);
    }

    shared_handle<tested_handle> h1{handle_construct, 1};
    check(h1, 1);

    { // self assignment
        h1 = h1;
        check(h1, 1);
    }

    {   // smart handle closure
        int closed_value = 0;
        {
            shared_handle<tested_handle> h2{handle_construct, 2};
            unsafe(h2).h.check_close = [&closed_value] (const tested_handle& closing_handle) noexcept
            {
                D_ASSERT(closed_value != 2 && closing_handle.value == 2);
                closed_value = closing_handle.value;
            };
        }
        D_ASSERT(closed_value == 2);
    }

    { // copy constructor
        shared_handle<tested_handle> h2{h1};
        check(h1, h2, 1);
    }
    check(h1, 1);

    { // assignment initialization 
        int h2_closed_value = 0;
        shared_handle<tested_handle> h2{handle_construct, 2};
        unsafe(h2).h.check_close = [&h2_closed_value] (const tested_handle& closing_handle) noexcept
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
        shared_handle<tested_handle> h2{handle_construct, 2};

        int h1_closed_value = 0;
        unsafe(h1).h.check_close = [&h1_closed_value] (const tested_handle& closing_handle) noexcept
        {
            D_ASSERT(h1_closed_value != 1 && closing_handle.value == 1);
            h1_closed_value = closing_handle.value;
        };

        D_ASSERT(h1->value == 1);
        h1 = h2;
        check(h1, h2, 2);
        D_ASSERT(h1_closed_value == 1);
    }
    check(h1, 2);
    unsafe(h1).h.value = 1;

    {   // cyclic assignment
        shared_handle<tested_handle> h2{h1};
        check(h1, h2, 1);
        h1 = h2;
        check(h1, h2, 1);
        h2 = h1;
        check(h1, h2, 1);
    }
    check(h1, 1);

    {   // cyclic assignment (ref count > 2)
        shared_handle<tested_handle> h2{h1};
        shared_handle<tested_handle> h3{h2};
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
            shared_handle<tested_handle> ch1{h1};
            check(h1, ch1, 1);

            shared_handle<tested_handle> h2{handle_construct, 2};
            shared_handle<tested_handle> ch2{h2};
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

            unsafe(h2).h.check_close = [&h2_closed_value] (const tested_handle& closing_handle) noexcept
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

        void operator () (const tested_handle& closing_handle) noexcept
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
