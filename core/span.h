#pragma once

#include <iterator>
#include <type_traits>
#include <numeric>

#include <core/narrow_cast.h>

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
        : data_{ data }
        , size_{ size }
    {
        assert( data || !size );
        assert( size <= max_size() );
    }

    template<class Container, class = decltype( std::data( std::declval<Container>() ) ), class = decltype( std::size( std::declval<Container>() ) )>
    constexpr span( Container& c ) noexcept
        : span{ std::data( c ), narrow_cast<size_type>( std::size( c ) ) }
    {}

    constexpr iterator begin() const noexcept
    {
        return data_;
    }

    constexpr iterator end() const noexcept
    {
        return data_ + size_;
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
        return size_;
    }

    constexpr size_type length() const noexcept
    {
        return size_;
    }

    constexpr bool empty() const noexcept
    {
        return !size_;
    }

    constexpr pointer data() const noexcept
    {
        return data_;
    }

    constexpr const_pointer cdata() const noexcept
    {
        return data();
    }

    constexpr reference value( size_type index ) const noexcept
    {
        assert( index < size_ );
        return data_[index];
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
        return value( size_ - 1_z );
    }

    constexpr span left( size_type size ) const noexcept
    {
        assert( size <= size_ );
        return { data_, size };
    }

    constexpr span right( size_type size ) const noexcept
    {
        assert( size <= size_ );
        return { data_ + size_ - size, size };
    }

    constexpr span mid( size_type pos, size_type size ) const noexcept
    {
        assert( pos <= size_ );
        assert( size <= ( size_ - pos ) );
        return { data_ + pos, size };
    }

    constexpr span suffix( size_type size ) const noexcept
    {
        return right( size );
    }

    constexpr span prefix( size_type size ) const noexcept
    {
        return left( size );
    }

    constexpr span sub( size_type pos, size_type size ) const noexcept
    {
        return mid( pos, size );
    }

    constexpr span without_prefix( size_type size ) const noexcept
    {
        assert( size <= size_ );
        return { data_ + size, size_ - size };
    }

    constexpr span without_suffix( size_type size ) const noexcept
    {
        assert( size <= size_ );
        return { data_, size_ - size };
    }

    constexpr bool in( span span ) const noexcept
    {
        return span.cbegin() >= cbegin() && span.cend() <= cend();
    }

    constexpr ptrdiff_t index( const_iterator position ) const noexcept
    {
        assert( position >= cbegin() && position <= cend() );
        return position - cbegin();
    }

    constexpr span<std::add_const_t<value_type>> cspan() const noexcept
    {
        return { data_, size_ };
    }

    static constexpr size_type max_size() noexcept
    { 
        return narrow_cast<size_type>( (std::numeric_limits<difference_type>::max)() );
    }

private:
    pointer data_{};
    size_type size_{};
};

#pragma warning(pop)
