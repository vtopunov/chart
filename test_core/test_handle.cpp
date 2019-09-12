#include <functional>

#include <core/safe_handle.h>
#include <core/assert.h>

void test_handle() noexcept
{
    struct test_handle
    {
    public:
        test_handle() noexcept = default;

        test_handle( int right ) noexcept
            : value{ right }
        {
            assert( value != closed_value );
        }

        ~test_handle() noexcept
        {
            if ( check_dtor )
            {
                check_dtor( value, closed_value );
            }
        }

        void close() noexcept
        {
            assert( check_dtor || check_close );
            assert( closed_value != value );
            closed_value = value;

            if ( check_close )
            {
                auto temp = std::move( check_close );
                check_close = {};
                temp( *this );
            }
        }

        constexpr bool is_valid() const noexcept
        {
            return value && value != closed_value;
        }

        int value = 0;
        int closed_value = -1;
        std::function<void( int, int )> check_dtor;
        std::function<void( test_handle& )> check_close;
    };

#pragma warning( push )
#pragma warning( disable : 26415 ) 
#pragma warning( disable : 26418 )
    constexpr struct
    {
        constexpr bool operator () ( const safe_handle<test_handle>& h1, int value ) const noexcept
        {
            assert( h1.is_unique() && h1.copies().next == &h1 && h1.copies().prev == &h1 );
            assert( h1->value == value );
            return true;
        }

        constexpr bool operator () ( const safe_handle<test_handle>& h1, const safe_handle<test_handle>& h2, int value ) const noexcept
        {
            assert( !h1.is_unique() && h1.copies().next == &h2 && h1.copies().prev == &h2 );
            assert( !h2.is_unique() && h2.copies().next == &h1 && h2.copies().prev == &h1 );
            assert( h1->value == value && h2->value == value );
            return true;
        }

        constexpr bool operator () ( const safe_handle<test_handle>& h1, const safe_handle<test_handle>& h2, const safe_handle<test_handle>& h3, int value ) const noexcept
        {
            assert( !h1.is_unique() && h1.copies().next == &h2 && h1.copies().prev == &h3 );
            assert( !h2.is_unique() && h2.copies().next == &h3 && h2.copies().prev == &h1  );
            assert( !h3.is_unique() && h3.copies().next == &h1 && h3.copies().prev == &h2  );
            assert( h1->value == value && h2->value == value && h3->value == value );
            return true;
        };
    } check;

    constexpr auto unsafe = [] ( const safe_handle<test_handle>& handle ) noexcept -> test_handle &
    {
        return const_cast<test_handle&>( handle.get() );
    };
#pragma warning(pop)

    {
        struct handle_without_is_valid
        {
            constexpr void close() noexcept 
            {}
        };

        struct handle_with_is_valid
        {
            constexpr void close() noexcept
            {}

            constexpr bool is_valid() const noexcept 
            {
                return false;
            }
        };
        
        safe_handle<handle_without_is_valid> safe_without_is_valid;
        safe_handle<handle_with_is_valid> safe_with_is_valid;

        assert( safe_without_is_valid );
        assert( !safe_with_is_valid );
    }

    safe_handle<test_handle> h1{ 1 };
    check( h1, 1 );

    { // self assignment
        h1 = h1;
        check( h1, 1 );
    }

    {   // smart handle closure
        bool h2_was_closed = false;
        {
            safe_handle<test_handle> h2{ 2 };
            unsafe( h2 ).check_dtor = [ &h2_was_closed ] ( int value, int closed_value ) noexcept
            {
                h2_was_closed = ( value == 2 && closed_value == value );
            };
        }
        assert( h2_was_closed );
    }

    { // copy constructor
        safe_handle<test_handle> h2{ h1 };
        check( h1, h2, 1 );
    }
    check( h1, 1 );

    { // assignment initialization 
        bool h2_was_closed = false;
        safe_handle<test_handle> h2;
        unsafe( h2 ).check_close = [ &h2_was_closed ] ( test_handle& closing_handle ) noexcept
        {
            h2_was_closed = ( closing_handle.value == 0 );
        };
        h2 = h1;
        check( h1, h2, 1 );
        assert( h2_was_closed );
        assert( !( h2->check_dtor ) );
    }
    check( h1, 1 );

    {   // assignment
        safe_handle<test_handle> h2{ 2 };

        bool h1_was_closed = false;
        unsafe( h1 ).check_close = [ &h1_was_closed ] ( test_handle& closing_handle ) noexcept
        {
            h1_was_closed = ( closing_handle.value == 1 );
        };

        h1 = h2;
        check( h1, h2, 2 );
        assert( h1_was_closed );
        assert( !( h1->check_dtor ) );
    }
    check( h1, 2 );
    unsafe( h1 ).value = 1;

    {   // cyclic assignment
        safe_handle<test_handle> h2{ h1 };
        check( h1, h2, 1 );
        h1 = h2;
        check( h1, h2, 1 );
        h2 = h1;
        check( h1, h2, 1 );
    }
    check( h1, 1 );

    {   // cyclic assignment (ref count > 2)
        safe_handle<test_handle> h2{ h1 };
        safe_handle<test_handle> h3{ h2 };
        check( h1, h2, h3, 1 );
        h2 = h3;
        check( h3, h2, h1, 1 );
        h3 = h2;
        check( h2, h3, h1, 1 );
        h1 = h2;
        check( h2, h1, h3, 1 );
    }
    check( h1, 1 );

    {   // assignment (ref count >= 2)
        safe_handle<test_handle> ch1{ h1 };
        check( h1, ch1, 1 );

        safe_handle<test_handle> h2{ 2 };
        safe_handle<test_handle> ch2{ h2 };
        check( h2, ch2, 2 );

        h2 = h1;
        check( ch1, h1, h2, 1 );
        check( ch2, 2 );

        ch1 = ch2;
        check( ch1, ch2, 2 );
        check( h1, h2, 1 );

        h2 = ch2;
        check( ch1, ch2, h2, 2 );
        check( h1, 1 );

        ch1 = h1;
        check( h2, ch2, 2 );
        check( h1, ch1, 1 );

        unsafe( h2 ).check_dtor = [] ( int value, int closed_value ) noexcept
        {
            assert( value == 2 && closed_value == value );
        };
    }

    check( h1, 1 );
    unsafe( h1 ).check_dtor = [] ( int value, int closed_value ) noexcept
    {
        assert( value == 1 && closed_value == value );
    };
}
