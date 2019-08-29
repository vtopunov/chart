#pragma once

#include <iterator>
#include <type_traits>
#include <algorithm>

#include "assert.h"

#pragma warning(push)
#pragma warning(disable : 26481) // Don't use pointer arithmetic. Use span instead

template <class T>
class span
{
public:
    using value_type             = T;
    using pointer                = T *;
    using const_pointer          = const T*;
    using reference              = T &;
    using const_reference        = const T &;
    using const_iterator         = const_pointer;
    using iterator               = pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;

    constexpr span() noexcept = default;

    constexpr span( pointer data, size_type size ) noexcept
        : span_data{ data }
        , span_size{ size }
    {
        assert( data || !size );
    }

    template<class Container, class = decltype( std::data( std::declval<Container>() ) ), class = decltype( std::size( std::declval<Container>() ) )>
    constexpr span( Container & c ) noexcept
        : span{ std::data( c ), std::size( c ) }
    {}

    constexpr iterator begin() const noexcept
    {
        return span_data;
    }

    constexpr iterator end() const noexcept
    {
        return span_data + span_size;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

    constexpr reverse_iterator rbegin() const noexcept
    {
        return reverse_iterator{ end() };
    }

    constexpr reverse_iterator rend() const noexcept
    {
        return reverse_iterator{ begin() };
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator{ end() };
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator{ begin() };
    }

    constexpr size_type size() const noexcept
    {
        return span_size;
    }

    constexpr size_type length() const noexcept
    {
        return span_size;
    }

    constexpr bool empty() const noexcept
    {
        return !span_size;
    }

    constexpr pointer data() const noexcept
    {
        return span_data;
    }

    constexpr reference value( size_type index ) const noexcept
    {
        assert( index < span_size );
        return span_data[index];
    }

    constexpr reference operator[]( size_type index ) const noexcept
    {
        return value( index );
    }

    constexpr reference front() const noexcept
    {
        return value( 0_z );
    }

    constexpr reference back() const noexcept
    {
        return value( span_size - 1_z );
    }

    constexpr span left( size_type size ) const noexcept
    {
        assert( size <= span_size );
        return { span_data, size };
    }

    constexpr span right( size_type size ) const noexcept
    {
        assert( size <= span_size );
        return { span_data + span_size - size, size };
    }

    constexpr span mid( size_type pos, size_type size ) const noexcept
    {
        assert( pos <= span_size );
        assert( size <= ( span_size - pos ) );
        return { span_data + pos, size };
    }

    constexpr bool in( const_iterator position ) const noexcept
    {
        return position >= cbegin() && position < cend();
    }

    constexpr bool in( span span ) const noexcept
    {
        return span.cbegin() >= cbegin() && span.cend() <= cend();
    }

    constexpr bool own( const_iterator position ) const noexcept
    {
        return position >= cbegin() && position <= cend();
    }

    constexpr iterator erase( iterator position ) noexcept
    {
        assert( in( position ) );
        const auto erased = std::move( std::next( position ), end(), position );
        --span_size;
        return erased;
    }

private:
    pointer span_data{};
    size_type span_size{};
};

#pragma warning(pop)
