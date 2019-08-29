#pragma once

#include <array>
#include <vector>
#include <iterator>
#include <valarray>

#include "util.h"
#include "span.h"

template<class iterator>
struct optional_iterator
{
    iterator position;
    bool has_value;

    constexpr iterator position_or( iterator other ) const noexcept
    {
        return ( has_value ) ? position : other;
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value;
    }

    constexpr operator iterator () const noexcept
    {
        return position;
    }

    constexpr iterator operator -> () const noexcept
    {
        return position;
    }

    constexpr decltype(auto) operator * () const noexcept
    {
        return *position;
    }
};

template<class key_type, class value_type, size_t static_size>
class static_map
{
public:
    struct element
    {
        key_type key;
        value_type value;
    };

    using pointer = element *;
    using const_pointer = const element*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using optional_item = optional_iterator<iterator>;
    using const_optional_item = optional_iterator<const_iterator>;

    struct element_key_ref
    {
        const key_type& key;

        constexpr element_key_ref( const key_type& key ) noexcept
            : key{ key }
        {}

        constexpr element_key_ref( const element& item ) noexcept
            : key{ item.key }
        {}
    };

    static constexpr auto less = [] ( element_key_ref left, element_key_ref right ) noexcept
    {
        return left.key < right.key;
    };

    static constexpr auto eq = [] ( element_key_ref left, element_key_ref right ) noexcept
    {
        return left.key == right.key;
    };

    const_iterator find( const key_type& key ) const noexcept
    {
        return mutable_this()->find( key );
    }

    const_optional_item item( const key_type& key ) const noexcept
    {
        const auto result = mutable_this()->item( key );
        return { result.position, result.has_value };
    }

    iterator find( const key_type& key ) noexcept
    {
        return item( key ).position_or( end() );
    }

    optional_item item( const key_type& key ) noexcept
    {
        const auto postion = _lower_bound( key );
        return { postion, _is_inserted( postion, key ) };
    }

    bool contains( const key_type& key ) const noexcept
    {
        return item( key ).has_value;
    }

    std::pair<iterator, bool> insert( element new_item ) noexcept
    {
        const auto position = item( new_item.key );
        if ( position )
        {
            return { position, false };
        }

        const auto result =
            _insert_hint( position, std::move( new_item ) );

        return { result, true };
    }

    std::pair<iterator, bool> insert( key_type key, value_type value ) noexcept
    {
        return insert( element{ std::move( key ), std::move( value ) } );
    }

    iterator insert_or_assign( element new_item ) noexcept
    {
        const auto position = item( new_item.key );
        if ( position )
        {
            *position = std::move( new_item );
            return position;
        }

        return _insert_hint( position, std::move( new_item ) );
    }

    iterator insert_or_assign( key_type key, value_type value ) noexcept
    {
        return insert_or_assign( element{ std::move( key ), std::move( value ) } );
    }

    iterator unsafe_insert( element new_item ) noexcept
    {
        const auto position = _lower_bound( new_item );
        return _insert_hint( position, std::move( new_item ) );
    }

    iterator unsafe_insert( key_type key, value_type value ) noexcept
    {
        return unsafe_insert( element{ std::move( key ), std::move( value ) } );
    }

    void erase( const key_type& key ) noexcept
    {
        if ( const auto position = item( key ) )
        {
            unsafe_erase( position );
        }
    }

    void erase( iterator position )
    {
        if ( data_.in( position ) )
        {
            unsafe_erase( position );
        }
    }

    void unsafe_erase( const key_type& key )
    {
        const auto position = _lower_bound( key );
        assert( _is_inserted( position, key ) );
        unsafe_erase( position );
    }

    void unsafe_erase( iterator position )
    {
        assert( data_.in( position ) );

        if ( data_.size() <= static_size )
        {
            data_.erase( position );
            return;
        }

        array_.dynamic_.erase( std::next( array_.dynamic_.begin(), position - data_.begin() ) );
        data_ = array_.dynamic_;

        if ( data_.size() <= static_size )
        {
            std::vector<element> dynamic{ std::move( array_.dynamic_ ) };
            std::uninitialized_move( dynamic.begin(), dynamic.end(), array_.static_.begin() );
            data_ = array_.static_;
        }
    }

    constexpr size_t size() const noexcept { return data_.size(); }

    constexpr iterator begin() noexcept { return data_.begin(); }

    constexpr iterator end() noexcept { return data_.end(); }

    constexpr const_iterator begin() const noexcept { return data_.begin(); }

    constexpr const_iterator end() const noexcept { return data_.end(); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }

    constexpr const_iterator cend() const noexcept { return end(); }

    constexpr bool is_static() const noexcept { return data_.size() <= static_size; }

    ~static_map() noexcept
    {
        if ( !is_static() )
        {
            array_.dynamic_.~vector<element>();
        }
    }

private:
    constexpr static_map* mutable_this() const noexcept
    {
        return as_mutable_pointer( this );
    }

    constexpr bool _is_inserted( const_iterator position, element_key_ref key ) const noexcept
    {
        return data_.in( position ) && eq( *position, key );
    }

    iterator _lower_bound( element_key_ref key ) noexcept
    {
        return std::lower_bound( begin(), end(), key, less );
    }

    iterator _insert_hint( iterator position, element item )
    {
        assert( data_.own( position ) );
        assert( !_is_inserted( position, item ) );

        if ( data_.size() < static_size )
        {
            std::move_backward( position, data_.end(), std::next( data_.end() ) );
            *position = std::move( item );
            data_ = { data_.data(), data_.size() + 1_z };
            return position;
        }

        const auto index = position - data_.begin();

        if ( data_.size() == static_size )
        {
            std::vector<element> dynamic;
            dynamic.reserve( 2 * array_.static_.size() );
            dynamic.assign( std::make_move_iterator( array_.static_.begin() ), std::make_move_iterator( array_.static_.end() ) );
            new ( &array_.dynamic_ ) std::vector<element>{ std::move( dynamic ) };
        }

        const auto result = array_.dynamic_.insert( std::next( array_.dynamic_.begin(), index ), std::move( item ) );
        data_ = array_.dynamic_;
        return &( *result );
    }

private:
    union static_or_dynamic_array
    {
        std::array<element, static_size> static_;
        std::vector<element> dynamic_;

        constexpr static_or_dynamic_array() noexcept
            : static_{ element{ {}, {} } }
        {}

        ~static_or_dynamic_array() noexcept {}
    } array_{};
    span<element> data_{ array_.static_.data(), size_t{} };
};
