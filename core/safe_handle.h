#pragma once

#include <core/util.h>

template<class handle_type>
class safe_handle
{
public:
    constexpr safe_handle( handle_type right = {} ) noexcept
        : handle_{ right }
        , prev_{ this }
        , next_{ this }
    {
        handle_.construct_weak( std::as_const( *this ) );
    }

    constexpr safe_handle( const safe_handle& right ) noexcept
        : handle_{ right.handle_ }
        , prev_{ &right }
        , next_{ right.next() }
    {
        as_mutable_pointer( next_ )->prev_ = this;
        as_mutable_pointer( prev_ )->next_ = this;
    }

    ~safe_handle() noexcept
    {
        if ( is_unique() )
        {
            handle_.close( std::as_const( *this ) );
        }
        else
        {
            const auto prev = prev_;
            const auto next = next_;
            as_mutable_pointer( prev )->next_ = next;
            as_mutable_pointer( next )->prev_ = prev;
            handle_.replace_weak( std::as_const( *this ), *prev );
        }
    }

    safe_handle& operator = ( const safe_handle& right ) noexcept
    {
        if ( this != &right )
        {
            if ( is_unique() )
            {
                handle_.close( std::as_const( *this ) );
            }
            else
            {
                const auto prev = prev_;
                const auto next = next_;
                as_mutable_pointer( prev )->next_ = next;
                as_mutable_pointer( next )->prev_ = prev;
            }

            handle_ = right.handle_;
            prev_ = &right;
            next_ = right.next();
            as_mutable_pointer( next_ )->prev_ = this;
            as_mutable_pointer( prev_ )->next_ = this;
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

    constexpr bool is_valid() const noexcept
    {
        return handle_.is_valid();
    }

    constexpr bool is_unique() const noexcept
    {
        return next_ == this;
    }

    constexpr const safe_handle* previous() const noexcept
    {
        return prev_;
    }

    constexpr const safe_handle* next() const noexcept
    {
        return next_;
    }

    constexpr const handle_type& get() const noexcept
    {
        return handle_;
    }

    struct enumerator_copies
    {
        const safe_handle* root;

        constexpr enumerator_copies begin() const noexcept { return *this; }
        constexpr enumerator_copies end() const noexcept { return *this; }
        constexpr enumerator_copies cbegin() const noexcept { return begin(); }
        constexpr enumerator_copies cend() const noexcept { return end(); }
    };

    constexpr enumerator_copies copies() const noexcept { return { this }; }

private:
    handle_type handle_;
    const safe_handle* prev_;
    const safe_handle* next_;
};