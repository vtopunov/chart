#include <set>
#include <vector>
#include <functional>

#include <core/small_flat_map.h>
#include <core/assert.h>

void test_small_flat_map() noexcept
{
    using map_ii7 = small_flat_map<int, int, 7>;
    map_ii7 map;

    static_assert( std::is_trivial_v<typename map_ii7::value_type> );

    auto check_contains = [&map = std::as_const( map )]( bool contains, int key, int value )
    {
        assert( std::is_sorted( map.cbegin(), map.cend(), map.less ) );
        assert( std::adjacent_find( map.cbegin(), map.cend(), map.eq ) == map.cend() );

        if ( map.is_static() )
        {
            assert( map.capacity() == map.static_size );
        }
        assert( map.size() <= map.capacity() );

        const auto size = map.size();
        assert( map.cbegin() == map.data() );
        assert( map.cend() == map.data() + size );

        assert( map.contains( key ) == contains );

        const auto item = map.item( key );
        assert( item.has_value == contains );
        const auto position = item.position;

        const auto find_it = map.find( key );
        const auto items = map.items( key );
        const auto count = map.count( key );

        if ( contains )
        {
            assert( size > 0_z );
            assert( position >= map.cbegin() && position < map.cend() );
            assert( position->key == key && position->value == value );
            assert( find_it == position );
            assert( items.data() == position && items.size() == 1_z );
            assert( count == 1_z );
        }
        else
        {
            assert( position >= map.cbegin() && position <= map.cend() );
            assert( find_it == map.cend() );
            assert( items.data() == position && items.size() == 0_z );
            assert( count == 0_z );
        }

        return position;
    };

    auto check_insert = [ &map, &check_contains ] ( int key, int value )
    {
        const auto size = map.size();
        const auto capacity = map.capacity();
        const auto is_static = map.is_static();
        const auto data = map.data();

        check_contains( false, key, value );
        const auto ins = map.insert( key, value );
        assert( ins.second );
        const auto pos = check_contains( true, key, value );
        assert( ins.first == pos );
        assert( map.size() == size + 1_z );

        if ( is_static != map.is_static() )
        {
            assert( is_static );
            assert( size == map.static_size );
            assert( map.data() != data );
            assert( map.capacity() > capacity );
        }
        else
        {
            if ( is_static )
            {
                assert( map.data() == data );
                assert( map.capacity() == capacity );
            }
            else
            {
                if ( map.data() != data )
                {
                    assert( map.capacity() > capacity );
                }
                else
                {
                    assert( map.capacity() == capacity );
                }
            }
        }

        return pos;
    };

    auto check_stabile = [&map = std::as_const( map )]( std::function<map_ii7::const_iterator( map_ii7& )> noise )
    {
        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();
        const std::vector< map_ii7::value_type > copy{ map.cbegin(), map.cend() };

        const auto result = noise( const_cast<map_ii7&>( map ) );

        assert( size == map.size() );
        assert( is_static == map.is_static() );
        assert( data == map.data() );
        assert( std::equal( copy.cbegin(), copy.cend(), map.cbegin() ) );

        return result;
    };

    auto check_insert_fail = [ &check_stabile, &check_contains ] ( int key, int value, int true_value )
    {
        return check_stabile( [ &check_contains, key, value, true_value ] ( map_ii7& map )
        {
            const auto pos = check_contains( true, key, true_value );
            const auto ins = map.insert( key, value );
            assert( !ins.second );
            assert( ins.first == pos );
            assert( ins.first->value == true_value );
            assert( check_contains( true, key, true_value ) == pos );
            return pos;
        } );
    };

    auto check_erase_fail = [ &check_stabile, &check_contains ] ( int key )
    {
        return check_stabile( [ &check_contains, key ] ( map_ii7& map )
        {
            check_contains( false, key, 0 );
            map.erase( key );
            return map.cbegin();
        } );
    };

    check_insert( 1, 1 );
    check_insert( 4, 16 );
    check_insert( 2, 4 );
    check_insert_fail( 2, 5, 4 );
    check_insert( 3, 9 );
    check_insert( 5, 25 );
    check_insert_fail( 5, 26, 25 );
    check_insert_fail( 3, 8, 9 );
    check_insert( 8, 64 );
    check_insert( 7, 49 );

    {
        assert( map.is_static() && map.size() == map.static_size );
        check_insert( 6, 36 );
        assert( !map.is_static() && map.size() == map.static_size + 1_z );
    }

    auto check_erase_back = [ &map, &check_contains ] ( int key, int value, int back_key, int back_value, int front_key, int front_value )
    {
        assert( map.size() >= 3 );
        assert( key > back_key );
        assert( back_key > front_key );

        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();

        assert( check_contains( true, key, value ) == std::prev( map.cend() ) );
        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend(), 2 ) );
        assert( check_contains( true, front_key, front_value ) == map.cbegin() );

        map.erase( key );

        assert( check_contains( true, front_key, front_value ) == map.cbegin() );
        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        check_contains( false, key, value );

        assert( map.cend()->key == key && map.cend()->value == value );

        assert( map.data() == data );
        assert( map.is_static() == is_static );
        assert( map.size() == size - 1_z );
    };

    auto check_erase_front = [ &map, &check_contains ] ( int key, int value, int front_key, int front_value, int back_key, int back_value )
    {
        assert( map.size() >= 3 );
        assert( key < front_key );
        assert( front_key < back_key );

        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();

        assert( check_contains( true, key, value ) == map.cbegin() );
        assert( check_contains( true, front_key, front_value ) == std::next( map.cbegin() ) );
        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );

        map.erase( key );

        assert( check_contains( true, front_key, front_value ) == map.cbegin() );
        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        check_contains( false, key, value );

        assert( map.cend()->key == back_key && map.cend()->value == back_value );

        assert( map.data() == data );
        assert( map.is_static() == is_static );
        assert( map.size() == size - 1_z );
    };

    auto check_erase_preback = [ &map, &check_contains ] ( int key, int value, int back_key, int back_value, int preback_key, int preback_value, int front_key, int front_value )
    {
        assert( map.size() >= 4 );
        assert( back_key > key );
        assert( key > preback_key );
        assert( preback_key > front_key );

        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();

        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        assert( check_contains( true, key, value ) == std::prev( map.cend(), 2 ) );
        assert( check_contains( true, preback_key, preback_value ) == std::prev( map.cend(), 3 ) );
        assert( check_contains( true, front_key, front_value ) == map.cbegin() );

        map.erase( key );

        assert( check_contains( true, front_key, front_value ) == map.cbegin() );
        assert( check_contains( true, preback_key, preback_value ) == std::prev( map.cend(), 2 ) );
        check_contains( false, key, value );
        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );

        assert( map.cend()->key == back_key && map.cend()->value == back_value );

        assert( map.data() == data );
        assert( map.is_static() == is_static );
        assert( map.size() == size - 1_z );
    };

    auto check_erase_prepreback = [&map, &check_contains](
        int key, int value,
        int back_key, int back_value,
        int preback_key, int preback_value,
        int prepreback_key, int prepreback_value,
        int front_key, int front_value )
    {
        assert( map.size() >= 5 );
        assert( back_key > preback_key );
        assert( preback_key > key );
        assert( key > prepreback_key );
        assert( prepreback_key > front_key );

        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();

        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        assert( check_contains( true, preback_key, preback_value ) == std::prev( map.cend(), 2 ) );
        assert( check_contains( true, key, value ) == std::prev( map.cend(), 3 ) );
        assert( check_contains( true, prepreback_key, prepreback_value ) == std::prev( map.cend(), 4 ) );
        assert( check_contains( true, front_key, front_value ) == map.cbegin() );

        map.erase( key );

        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        assert( check_contains( true, preback_key, preback_value ) == std::prev( map.cend(), 2 ) );
        check_contains( false, key, value );
        assert( check_contains( true, prepreback_key, prepreback_value ) == std::prev( map.cend(), 3 ) );
        assert( check_contains( true, front_key, front_value ) == map.cbegin() );

        assert( map.cend()->key == back_key && map.cend()->value == back_value );

        assert( map.data() == data );
        assert( map.is_static() == is_static );
        assert( map.size() == size - 1_z );
    };

    auto check_shrink_to_static = [ &map, &check_contains ] ( int back_key, int back_value, int front_key, int front_value )
    {
        assert( map.size() == map.static_size );
        assert( map.capacity() > map.static_size );
        assert( !map.is_static() );
        assert( back_key > front_key );

        const auto size = map.size();
        const auto data = map.data();
        const std::vector<map_ii7::value_type > copy( map.cbegin(), map.cend() );

        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        assert( check_contains( true, front_key, front_value ) == map.cbegin() );

        map.shrink_to_fit();

        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        assert( check_contains( true, front_key, front_value ) == map.cbegin() );

        assert( map.data() != data );
        assert( map.is_static() );
        assert( map.capacity() == map.static_size );
        assert( map.size() == size );
        assert( std::equal( copy.cbegin(), copy.cend(), map.cbegin() ) );
    };

    auto check_shrink_to_fit_dynamic = [ &map, &check_contains ] ( int back_key, int back_value, int front_key, int front_value )
    {
        assert( map.size() > map.static_size );
        assert( map.capacity() > map.size() );
        assert( !map.is_static() );
        assert( back_key > front_key );

        const auto size = map.size();
        const auto is_static = map.is_static();

        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        assert( check_contains( true, front_key, front_value ) == map.cbegin() );

        map.shrink_to_fit();

        assert( check_contains( true, back_key, back_value ) == std::prev( map.cend() ) );
        assert( check_contains( true, front_key, front_value ) == map.cbegin() );

        assert( map.is_static() == is_static );
        assert( map.capacity() == map.size() );
        assert( map.size() == size );
    };

    {
        assert( !map.is_static() && map.size() == map.static_size + 1_z );
        check_erase_back( 8, 64, 7, 49, 1, 1 );
        check_shrink_to_static( 7, 49, 1, 1 );
        check_insert( 8, 64 );
    }

    {
        assert( !map.is_static() && map.size() == map.static_size + 1_z );
        check_erase_preback( 7, 49, 8, 64, 6, 36, 1, 1 );
        check_shrink_to_static( 8, 64, 1, 1 );
        check_insert( 7, 49 );
    }

    {
        assert( !map.is_static() && map.size() == map.static_size + 1_z );
        check_erase_front( 1, 1, 2, 4, 8, 64 );
        check_shrink_to_static( 8, 64, 2, 4 );
        check_insert( 1, 1 );
    }

    {
        assert( !map.is_static() && map.size() == map.static_size + 1_z );
        check_insert( 9, 81 );
        check_erase_prepreback( 7, 49, 9, 81, 8, 64, 6, 36, 1, 1 );
        check_shrink_to_fit_dynamic( 9, 81, 1, 1 );
    }

    {
        assert( !map.is_static() && map.size() == map.static_size + 1_z );
        check_erase_fail( -1 );
        check_erase_fail( 10 );
        check_erase_fail( 7 );
    }

    {
        assert( !map.is_static() && map.size() == map.static_size + 1_z );
        check_erase_prepreback( 6, 36, 9, 81, 8, 64, 5, 25, 1, 1 );
        check_shrink_to_static( 9, 81, 1, 1 );
        check_erase_prepreback( 5, 25, 9, 81, 8, 64, 4, 16, 1, 1 );
        check_erase_front( 1, 1, 2, 4, 9, 81 );
        check_erase_back( 9, 81, 8, 64, 2, 4 );
        check_erase_fail( 1 );
        check_erase_fail( 9 );
        check_erase_fail( 5 );
        assert( map.is_static() );
    }

    {
        assert( map.count( 4 ) == 1_z );
        map.force_insert( 4, 17 );
        map.force_insert( 4, 18 );
        map.force_insert( 4, 19 );
        const auto items = map.items( 4 );
        constexpr map_ii7::value_type check[]
        {
            { 4, 16 },
            { 4, 17 },
            { 4, 18 },
            { 4, 19 }
        };
        assert( items.size() == std::size( check ) );
        assert( std::equal( items.begin(), items.end(), check ) );
        assert( map.erase( 4 ) == std::size( check ) );
        check_contains( false, 4, 16 );
    }

    {
        struct checker
        {
            static std::set<int>& for_destroy() noexcept
            {
                static std::set<int> for_destroy_;
                return for_destroy_;
            }

            static int unique_id() noexcept
            {
                static int id = 0;
                return ++id;
            }

            int i{ 0 };
            mutable int id = 0;

            checker() = default;

            checker( int i ) noexcept
                : i{ i }
            {}

            ~checker()
            {
                if ( id )
                {
                    assert( for_destroy().erase( id ) == 1_z );
                }
            }

            checker( const checker& right ) noexcept
                : i{ right.i }
            {
                assert( true );
            }

            checker& operator = ( const checker& ) noexcept
            {
                assert( true );
                return *this;
            }

            checker( checker&& right ) noexcept
                : i{ right.release_i() }
                , id{ right.release_id() }
            {}

            checker& operator = ( checker&& right ) noexcept
            {
                std::swap( i, right.i );
                std::swap( id, right.id );
                return *this;
            }

            bool operator < ( const checker& right ) const noexcept
            {
                return i < right.i;
            }

            bool operator == ( const checker& right ) const noexcept
            {
                return i == right.i;
            }

            int release_i() noexcept
            {
                int temp = i;
                i = 0;
                return temp;
            }

            int release_id() noexcept
            {
                int temp = id;
                id = 0;
                return temp;
            }

            void enable_check_destroy() const
            {
                assert( !id );
                id = unique_id();
                assert( for_destroy().insert( id ).second );
            }
        };

        small_flat_map<checker, checker, 4> checker_map;
        auto insert = [ &checker_map ] ( int key, int value )
        {
            const auto ins = checker_map.insert( key, value );
            assert( ins.second );
            ins.first->key.enable_check_destroy();
            ins.first->value.enable_check_destroy();
        };

        auto erase = [ &checker_map ] ( int key )
        {
            const auto items = checker_map.items( key );
            assert( items.size() == 1_z );

            const auto size = checker::for_destroy().size();
            const auto key_id = items.front().key.id;
            const auto value_id = items.front().value.id;

            assert( size >= 2_z );
            assert( key_id && checker::for_destroy().contains( key_id ) );
            assert( value_id && checker::for_destroy().contains( value_id ) );
            checker_map.erase( key );
            assert( checker::for_destroy().size() == size - 2_z );
            assert( !checker::for_destroy().contains( key_id ) );
            assert( !checker::for_destroy().contains( value_id ) );
        };

        insert( 1, 1 );
        insert( 4, 16 );
        insert( 2, 4 );
        assert( !checker_map.insert( 2, 5 ).second );
        insert( 6, 36 );
        insert( 10, 100 );
        insert( 7, 49 );
        erase( 7 );
        erase( 6 );
        checker_map.erase( 5 );
        erase( 4 );
        checker_map.erase( 3 );
        erase( 2 );
        erase( 1 );
        erase( 10 );
    }
}
