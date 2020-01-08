#pragma once

#include <core/util.h>
#include <core/intrusive_list.h>
#include <core/member_detector.h>

namespace safe_handle_private_detail
{
    template<class T>
    using has_replace_owwer_t = decltype( std::declval<T>().replace_owwer( nullptr ), 0 );

    template<class T>
    using has_is_valid_t = decltype( std::declval<T>().is_valid() );

    template<class T>
    using has_view_t = decltype( std::declval<T>().view() );

    template<class T>
    constexpr bool has_replace_owwer_v = is_detected_v<has_replace_owwer_t, T>;

    template<class T>
    constexpr bool has_is_valid_v = is_detected_v<has_is_valid_t, T>;

    template<class T>
    constexpr bool has_view_v = is_detected_v<has_view_t, T>;


    template<class T>
    constexpr decltype( auto ) view( const T& handle ) noexcept
    {
        if constexpr ( has_view_v<T> )
        {
            return handle.view();
        }
        else
        {
            constexpr struct {} private_view;
            return private_view;
        }
    }
}

template<class handle_type >
class safe_handle
{
public:
    using view_type = decltype(safe_handle_private_detail::view(std::declval<handle_type>()));

    constexpr safe_handle(handle_type right = {} ) noexcept
        : handle_{ right }
        , copies_{ this, this }
    {}

    constexpr safe_handle( const safe_handle& right ) noexcept
        : handle_{ right.handle_ }
        , copies_{ copies_impl_.push( this, const_cast<safe_handle*>( &right ) ) }
    {}

    ~safe_handle() noexcept
    {
        using safe_handle_private_detail::has_replace_owwer_v;

        if ( is_unique() )
        {
            handle_.close();
        }
        else
        {
            copies_impl_.pop( this );

            if constexpr ( has_replace_owwer_v<handle_type> )
            {
                handle_.replace_owwer( copies_.prev );
            }
        }
    }

    safe_handle& operator = ( const safe_handle& right ) noexcept
    {
        if ( this != &right )
        {
            if ( is_unique() )
            {
                handle_.close();
            }
            else
            {
                copies_impl_.pop( this );
            }

            handle_ = right.handle_;
            copies_ = copies_impl_.push( this, const_cast<safe_handle*>( &right ) );
        }

        return *this;
    }

    constexpr const handle_type* operator ->() const noexcept
    {
        assert( is_valid() );
        return &handle_;
    }

    explicit constexpr operator bool() const noexcept
    {
        return is_valid();
    }

    constexpr operator view_type () const noexcept
    {
        return safe_handle_private_detail::view( handle_ );
    }

    constexpr const handle_type& get() const noexcept
    {
        return handle_;
    }

    constexpr bool is_valid() const noexcept
    {
        using safe_handle_private_detail::has_is_valid_v;

        if constexpr ( has_is_valid_v<handle_type> )
        {
            return handle_.is_valid();
        }
        else
        {
            return true;
        }
    }

    constexpr bool is_unique() const noexcept
    {
        return copies_impl_.is_unique( this );
    }

    constexpr intrusive_node<safe_handle> copies() const noexcept
    {
        return copies_;
    }

private:
    handle_type handle_;
    intrusive_node<safe_handle> copies_;

    static constexpr intrusive_list_impl<safe_handle> copies_impl_{ &safe_handle::copies_ };
};


