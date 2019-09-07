#pragma once

#include <vector>
#include <iterator>

#include <core/util.h>
#include <core/span.h>

template<class iterator>
struct optional_iterator
{
    iterator position;
    bool has_value;

    constexpr iterator position_or( iterator other ) const noexcept
    {
        return ( has_value ) ? position : other;
    }

    explicit constexpr operator bool() const noexcept
    {
        return has_value;
    }

    constexpr iterator operator -> () const noexcept
    {
        assert( has_value );
        return position;
    }

    constexpr decltype( auto ) operator * () const noexcept
    {
        assert( has_value );
        return *position;
    }
};

template<class K, class T, size_t N>
class static_map
{
public:
    static constexpr size_t static_size = N;

    using key_type = K;
    using key_view = key_type;
    using mapped_type = T;

    struct value_type
    {
        key_type key;
        mapped_type value;

        constexpr operator key_view () const noexcept
        {
            return key;
        }
    };

    using pointer = value_type *;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using optional_item = optional_iterator<iterator>;
    using const_optional_item = optional_iterator<const_iterator>;

    static constexpr auto less = [] ( key_view left, key_view right ) noexcept
    {
        return left < right;
    };

    static constexpr auto eq = [] ( key_view left, key_view right ) noexcept
    {
        return left == right;
    };

    constexpr static_map() noexcept {}

    const_iterator find( key_view key ) const noexcept
    {
        return mutable_this()->find( key );
    }

    const_optional_item item( key_view key ) const noexcept
    {
        const auto result = mutable_this()->item( key );
        return { as_const_pointer( result.position ), result.has_value };
    }

    iterator find( key_view key ) noexcept
    {
        return item( key ).position_or( end() );
    }

    optional_item item( key_view key ) noexcept
    {
        const auto position = _lower_bound( key );
        return { position, position < cend() && eq( *position, key ) };
    }

    span<value_type> items( key_view key ) noexcept
    {
        const auto postion = _lower_bound( key );
        return { postion, count( postion, key ) };
    }

    span<const value_type> items( key_view key ) const noexcept
    {
        return mutable_this()->items( key ).cspan();
    }

    size_t count( const_iterator position, key_view key ) const noexcept
    {
        assert( _is_position( position ) );
        return std::count_if( position, cend(), [ key ] ( key_view item ) { return eq( item, key ); } );
    }

    size_t count( const_iterator position ) const noexcept
    {
        assert( _is_position( position ) );
        return count( position, position->key );
    }

    size_t count( key_view key ) const noexcept
    {
        return items( key ).size();
    }

    bool contains( key_view key ) const noexcept
    {
        return item( key ).has_value;
    }

    std::pair<iterator, bool> insert( value_type value ) noexcept
    {
        const auto position = item( value.key );
        if ( position )
        {
            return { position.position, false };
        }

        const auto result =
            _insert_hint( position.position, std::move( value ) );

        return { result, true };
    }

    std::pair<iterator, bool> insert( key_type key, mapped_type value ) noexcept
    {
        return insert( value_type{ std::move( key ), std::move( value ) } );
    }

    iterator force_insert( value_type new_item ) noexcept
    {
        const auto position = _lower_bound( new_item );
        return _insert_hint( position, std::move( new_item ) );
    }

    iterator force_insert( key_type key, mapped_type value ) noexcept
    {
        return force_insert( value_type{ std::move( key ), std::move( value ) } );
    }

    size_t erase( key_view key ) noexcept
    {
        const auto values = items( key );
        erase( values.begin(), values.end() );
        return values.size();
    }

    void erase( iterator position ) noexcept
    {
        erase( position, std::next( position ) );
    }

    void erase( iterator begin, iterator end ) noexcept
    {
        assert( end >= begin && begin >= data_.cbegin() && end <= data_.cend() );

        if ( is_static() )
        {
            std::destroy( std::move( end, data_.end(), begin ), data_.end() );
            data_ = data_.without_suffix( narrow_cast<size_t>( std::distance(begin, end) ) );
            return;
        }

        dynamic_.erase(
            std::next( dynamic_.cbegin(), data_.index( begin ) ),
            std::next( dynamic_.cbegin(), data_.index( end ) )
        );
        data_ = dynamic_;
    }

    void shrink_to_fit() noexcept
    {
        if ( !is_static() )
        {
            if ( size() <= static_size )
            {
                _switch_to_static();
            }
            else
            {
                dynamic_.shrink_to_fit();
                data_ = dynamic_;
            }
        }
    }

    constexpr size_t size() const noexcept { return data_.size(); }

    constexpr size_t capacity() const noexcept { return ( is_static() ) ? static_size : dynamic_.capacity(); }

    constexpr const_iterator data() const noexcept { return data_.data(); }

    constexpr iterator begin() noexcept { return data_.begin(); }

    constexpr iterator end() noexcept { return data_.end(); }

    constexpr const_iterator begin() const noexcept { return data_.cbegin(); }

    constexpr const_iterator end() const noexcept { return data_.cend(); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }

    constexpr const_iterator cend() const noexcept { return end(); }

    constexpr bool is_static() const noexcept
    {
        return cbegin() == std::cbegin( static_ );
    }

    ~static_map() noexcept
    {
        if ( is_static() )
        {
            _destroy_static();
        }
        else
        {
            _destroy_dynamic();
        }
    }

private:
    using dynarray_type = std::vector<value_type>;

    constexpr static_map* mutable_this() const noexcept
    {
        return as_mutable_pointer( this );
    }

    void _destroy_static() noexcept
    {
        std::destroy( begin(), end() );
    }

    void _destroy_dynamic() noexcept
    {
        std::destroy_at( &( dynamic_ ) );
    }

    void _switch_to_static() noexcept
    {
        dynarray_type dynamic{ std::move( dynamic_ ) };
        _destroy_dynamic();
        data_ = { std::data( static_ ), dynamic.size() };
        std::uninitialized_move( dynamic.begin(), dynamic.end(), begin() );
    }

    void _switch_to_dynamic() noexcept
    {
        dynarray_type dynamic;
        dynamic.reserve( 2 * size() );
        dynamic.assign( std::make_move_iterator( begin() ), std::make_move_iterator( end() ) );
        _destroy_static();
        new ( &dynamic_ ) dynarray_type( std::move( dynamic ) );
        data_ = dynamic_;
    }

    iterator _lower_bound( key_view key ) noexcept
    {
        return std::lower_bound( begin(), end(), key, less );
    }

    constexpr bool _is_position( const_iterator position ) const noexcept
    {
        return position >= cbegin() && position <= cend();
    }

    iterator _insert_hint( iterator position, value_type item )
    {
        assert( _is_position( position ) );

        const auto index = data_.index( position );

        if ( is_static() )
        {
            if ( size() < static_size )
            {
                std::move_backward( position, data_.end(), std::uninitialized_default_construct_n( data_.end(), 1_z ) );
                *position = std::move( item );
                data_ = { data_.data(), data_.size() + 1_z };
                return position;
            }

            _switch_to_dynamic();
        }

        const auto result = dynamic_.insert( std::next( dynamic_.cbegin(), index ), std::move( item ) );
        data_ = dynamic_;
        return &( *result );
    }

private:
    union
    {
        value_type static_[static_size];
        dynarray_type dynamic_;
    };
    span<value_type> data_{ std::data( static_ ), 0_z };
};
