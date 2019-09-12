#pragma once

template<class T>
struct intrusive_node
{
    T* prev;
    T* next;
};

template<class T>
struct intrusive_list_impl
{
    intrusive_node<T> T::*const plist;

    constexpr const intrusive_node<T> list( const T* item ) const noexcept
    {
        return ( item->*plist );
    }

    constexpr intrusive_node<T>& list_ref( T* item ) const noexcept
    {
        return ( item->*plist );
    }

    constexpr T*const next( const T* item ) const noexcept
    {
        return list( item ).next;
    }

    constexpr T*const prev( const T* item ) const noexcept
    {
        return list( item ).prev;
    }

    constexpr void set_next( T* item, T* next ) const noexcept
    {
        list_ref( item ).next = next;
    }

    constexpr void set_prev( T* item, T* prev ) const noexcept
    {
        list_ref( item ).prev = prev;
    }

    constexpr void pop( intrusive_node<T> item ) const noexcept
    {
        set_next( item.prev, item.next );
        set_prev( item.next, item.prev );
    }

    constexpr void pop( const T* item ) const noexcept
    {
        pop( list( item ) );
    }

    constexpr intrusive_node<T> push( T* current, T* item ) const noexcept
    {
        const intrusive_node<T> root{ item, next( item ) };
        set_next( root.prev, current );
        set_prev( root.next, current );
        return root;
    }

    template<class T>
    constexpr bool is_unique( const T* item ) const noexcept
    {
        return item == next( item );
    }
};



