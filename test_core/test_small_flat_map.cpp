#include <set>
#include <functional>

#include <core/small_flat_map.h>
#include <core/assert.h>

void test_small_flat_map() noexcept
{
    D_ASSERT( !"add recursion erase test" );

    using map_ii7 = small_flat_map<int, int, 7>;
    map_ii7 map;

    static_assert(std::is_trivial_v<typename map_ii7::value_type>);

    auto check_contains = [&map = std::as_const(map)](bool contains, int key, int value) noexcept
    {
        D_ASSERT( std::is_sorted( map.cbegin(), map.cend(), std::less<int>() ) );
        D_ASSERT( std::adjacent_find( map.cbegin(), map.cend(), std::equal_to<int>() ) == map.cend() );

        if ( map.is_static() )
        {
            D_ASSERT(map.capacity() == map.static_size);
        }
        D_ASSERT(map.size() <= map.capacity());

        const auto size = map.size();
        D_ASSERT(map.cbegin() == map.data());
        D_ASSERT(map.cend() == map.data() + size);

        D_ASSERT(map.contains(key) == contains);

        const auto l_position = map.unsafe_lower_bound(key);
        const auto u_position = map.unsafe_upper_bound(key);
        const auto find_it = map.find(key);
        const auto items = map.items(key);
        const auto count = map.count(key);

        size_t test_count = 0;
        map_ii7::const_iterator test_it = nullptr;
        for (auto it = map.cbegin(); it < map.cend(); ++it)
        {
            if (it->key == key)
            {
                D_ASSERT(!test_count);
                ++test_count;

                D_ASSERT(!test_it);
                test_it = it;
            }
        }

        D_ASSERT(l_position >= map.cbegin() && l_position <= map.cend());
        D_ASSERT(u_position >= l_position && u_position <= map.cend());
        D_ASSERT(find_it >= l_position && find_it <= map.cend());
        D_ASSERT(items.key_ == key);
        D_ASSERT(items.position_ == l_position);
        D_ASSERT(items.end_position_ == map.cend());
        D_ASSERT(test_it <= l_position);
        D_ASSERT(map.count(find_it) == count);
        D_ASSERT(map.count(find_it, key) == count);
        D_ASSERT(map.count(l_position, key) == count);
        D_ASSERT(test_count == count);
        D_ASSERT(count <= 1u);

        if (contains)
        {
            D_ASSERT(size > 0u);
            D_ASSERT(l_position < map.cend());
            D_ASSERT(l_position->key == key && l_position->value == value);
            D_ASSERT(u_position == std::next(l_position));
            D_ASSERT(find_it == l_position);
            D_ASSERT(test_it == l_position);
            D_ASSERT(count == 1u);
        }
        else
        {
            D_ASSERT(find_it == map.cend());
            D_ASSERT(!test_it);
            D_ASSERT(!count);
        }

        return l_position;
    };

    auto check_insert = [&map, &check_contains](int key, int value) noexcept
    {
        const auto size = map.size();
        const auto capacity = map.capacity();
        const auto is_static = map.is_static();
        const auto data = map.data();

        check_contains(false, key, value);
        const auto ins = map.insert(key, value);
        D_ASSERT(ins.second);
        const auto pos = check_contains(true, key, value);
        D_ASSERT(ins.first == pos);
        D_ASSERT(map.size() == size + 1u);

        if (is_static != map.is_static())
        {
            D_ASSERT(is_static);
            D_ASSERT(size == map.static_size);
            D_ASSERT(map.data() != data);
            D_ASSERT(map.capacity() > capacity);
        }
        else
        {
            if (is_static)
            {
                D_ASSERT(map.data() == data);
                D_ASSERT(map.capacity() == capacity);
            }
            else
            {
                if (map.data() != data)
                {
                    D_ASSERT(map.capacity() > capacity);
                }
                else
                {
                    D_ASSERT(map.capacity() == capacity);
                }
            }
        }

        return pos;
    };

    auto check_stabile = [&map = std::as_const(map)](std::function<map_ii7::const_iterator(map_ii7&)> noise) noexcept
    {
        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();
        const std::vector< map_ii7::value_type > copy{ map.cbegin(), map.cend() };

        const auto result = noise(const_cast<map_ii7&>(map));

        D_ASSERT(size == map.size());
        D_ASSERT(is_static == map.is_static());
        D_ASSERT(data == map.data());
        D_ASSERT(std::equal(copy.cbegin(), copy.cend(), map.cbegin()));

        return result;
    };

    auto check_insert_fail = [&check_stabile, &check_contains](int key, int value, int true_value) noexcept
    {
        return check_stabile([&check_contains, key, value, true_value](map_ii7& map) noexcept
            {
                const auto pos = check_contains(true, key, true_value);
                const auto ins = map.insert(key, value);
                D_ASSERT(!ins.second);
                D_ASSERT(ins.first == pos);
                D_ASSERT(ins.first->value == true_value);
                D_ASSERT(check_contains(true, key, true_value) == pos);
                return pos;
            });
    };

    auto check_erase_fail = [&check_stabile, &check_contains](int key) noexcept
    {
        return check_stabile([&check_contains, key](map_ii7& map) noexcept
            {
                check_contains(false, key, 0);
                map.erase(key);
                return map.cbegin();
            });
    };

    check_insert(1, 1);
    check_insert(4, 16);
    check_insert(2, 4);
    check_insert_fail(2, 5, 4);
    check_insert(3, 9);
    check_insert(5, 25);
    check_insert_fail(5, 26, 25);
    check_insert_fail(3, 8, 9);
    check_insert(8, 64);
    check_insert(7, 49);

    {
        D_ASSERT(map.is_static() && map.size() == map.static_size);
        check_insert(6, 36);
        D_ASSERT(!map.is_static() && map.size() == map.static_size + 1u);
    }

    auto check_erase_back = [&map, &check_contains](int key, int value, int back_key, int back_value, int front_key, int front_value) noexcept
    {
        D_ASSERT(map.size() >= 3);
        D_ASSERT(key > back_key);
        D_ASSERT(back_key > front_key);

        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();

        D_ASSERT(check_contains(true, key, value) == std::prev(map.cend()));
        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend(), 2));
        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());

        D_ASSERT(map.erase(key) == 1u);

        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());
        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        check_contains(false, key, value);

        D_ASSERT(map.cend()->key == key && map.cend()->value == value);

        D_ASSERT(map.data() == data);
        D_ASSERT(map.is_static() == is_static);
        D_ASSERT(map.size() == size - 1u);
    };

    auto check_erase_front = [&map, &check_contains](int key, int value, int front_key, int front_value, int back_key, int back_value) noexcept
    {
        D_ASSERT(map.size() >= 3);
        D_ASSERT(key < front_key);
        D_ASSERT(front_key < back_key);

        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();

        D_ASSERT(check_contains(true, key, value) == map.cbegin());
        D_ASSERT(check_contains(true, front_key, front_value) == std::next(map.cbegin()));
        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));

        D_ASSERT(map.erase(key) == 1u);

        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());
        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        check_contains(false, key, value);

        D_ASSERT(map.cend()->key == back_key && map.cend()->value == back_value);

        D_ASSERT(map.data() == data);
        D_ASSERT(map.is_static() == is_static);
        D_ASSERT(map.size() == size - 1u);
    };

    auto check_erase_preback = [&map, &check_contains](int key, int value, int back_key, int back_value, int preback_key, int preback_value, int front_key, int front_value) noexcept
    {
        D_ASSERT(map.size() >= 4);
        D_ASSERT(back_key > key);
        D_ASSERT(key > preback_key);
        D_ASSERT(preback_key > front_key);

        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();

        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        D_ASSERT(check_contains(true, key, value) == std::prev(map.cend(), 2));
        D_ASSERT(check_contains(true, preback_key, preback_value) == std::prev(map.cend(), 3));
        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());

        D_ASSERT(map.erase(key) == 1u);

        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());
        D_ASSERT(check_contains(true, preback_key, preback_value) == std::prev(map.cend(), 2));
        check_contains(false, key, value);
        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));

        D_ASSERT(map.cend()->key == back_key && map.cend()->value == back_value);

        D_ASSERT(map.data() == data);
        D_ASSERT(map.is_static() == is_static);
        D_ASSERT(map.size() == size - 1u);
    };

    auto check_erase_prepreback = [&map, &check_contains](
        int key, int value,
        int back_key, int back_value,
        int preback_key, int preback_value,
        int prepreback_key, int prepreback_value,
        int front_key, int front_value) noexcept
    {
        D_ASSERT(map.size() >= 5);
        D_ASSERT(back_key > preback_key);
        D_ASSERT(preback_key > key);
        D_ASSERT(key > prepreback_key);
        D_ASSERT(prepreback_key > front_key);

        const auto size = map.size();
        const auto is_static = map.is_static();
        const auto data = map.data();

        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        D_ASSERT(check_contains(true, preback_key, preback_value) == std::prev(map.cend(), 2));
        D_ASSERT(check_contains(true, key, value) == std::prev(map.cend(), 3));
        D_ASSERT(check_contains(true, prepreback_key, prepreback_value) == std::prev(map.cend(), 4));
        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());

        D_ASSERT(map.erase(key) == 1u);

        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        D_ASSERT(check_contains(true, preback_key, preback_value) == std::prev(map.cend(), 2));
        check_contains(false, key, value);
        D_ASSERT(check_contains(true, prepreback_key, prepreback_value) == std::prev(map.cend(), 3));
        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());

        D_ASSERT(map.cend()->key == back_key && map.cend()->value == back_value);

        D_ASSERT(map.data() == data);
        D_ASSERT(map.is_static() == is_static);
        D_ASSERT(map.size() == size - 1u);
    };

    auto check_shrink_to_static = [&map, &check_contains](int back_key, int back_value, int front_key, int front_value) noexcept
    {
        D_ASSERT(map.size() == map.static_size);
        D_ASSERT(map.capacity() > map.static_size);
        D_ASSERT(!map.is_static());
        D_ASSERT(back_key > front_key);

        const auto size = map.size();
        const auto data = map.data();
        const std::vector<map_ii7::value_type > copy(map.cbegin(), map.cend());

        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());

        map.shrink_to_fit();

        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());

        D_ASSERT(map.data() != data);
        D_ASSERT(map.is_static());
        D_ASSERT(map.capacity() == map.static_size);
        D_ASSERT(map.size() == size);
        D_ASSERT(std::equal(copy.cbegin(), copy.cend(), map.cbegin()));
    };

    auto check_shrink_to_fit_dynamic = [&map, &check_contains](int back_key, int back_value, int front_key, int front_value) noexcept
    {
        D_ASSERT(map.size() > map.static_size);
        D_ASSERT(map.capacity() > map.size());
        D_ASSERT(!map.is_static());
        D_ASSERT(back_key > front_key);

        const auto size = map.size();
        const auto is_static = map.is_static();

        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());

        map.shrink_to_fit();

        D_ASSERT(check_contains(true, back_key, back_value) == std::prev(map.cend()));
        D_ASSERT(check_contains(true, front_key, front_value) == map.cbegin());

        D_ASSERT(map.is_static() == is_static);
        D_ASSERT(map.capacity() == map.size());
        D_ASSERT(map.size() == size);
    };

    {
        D_ASSERT(!map.is_static() && map.size() == map.static_size + 1u);
        check_erase_back(8, 64, 7, 49, 1, 1);
        check_shrink_to_static(7, 49, 1, 1);
        check_insert(8, 64);
    }

    {
        D_ASSERT(!map.is_static() && map.size() == map.static_size + 1u);
        check_erase_preback(7, 49, 8, 64, 6, 36, 1, 1);
        check_shrink_to_static(8, 64, 1, 1);
        check_insert(7, 49);
    }

    {
        D_ASSERT(!map.is_static() && map.size() == map.static_size + 1u);
        check_erase_front(1, 1, 2, 4, 8, 64);
        check_shrink_to_static(8, 64, 2, 4);
        check_insert(1, 1);
    }

    {
        D_ASSERT(!map.is_static() && map.size() == map.static_size + 1u);
        check_insert(9, 81);
        check_erase_prepreback(7, 49, 9, 81, 8, 64, 6, 36, 1, 1);
        check_shrink_to_fit_dynamic(9, 81, 1, 1);
    }

    {
        D_ASSERT(!map.is_static() && map.size() == map.static_size + 1u);
        check_erase_fail(-1);
        check_erase_fail(10);
        check_erase_fail(7);
    }

    {
        D_ASSERT(!map.is_static() && map.size() == map.static_size + 1u);
        check_erase_prepreback(6, 36, 9, 81, 8, 64, 5, 25, 1, 1);
        check_shrink_to_static(9, 81, 1, 1);
        check_erase_prepreback(5, 25, 9, 81, 8, 64, 4, 16, 1, 1);
        check_erase_front(1, 1, 2, 4, 9, 81);
        check_erase_back(9, 81, 8, 64, 2, 4);
        check_erase_fail(1);
        check_erase_fail(9);
        check_erase_fail(5);
        D_ASSERT(map.is_static());
    }

    {
        D_ASSERT(map.count(4) == 1u);
        map.force_insert(4, 17);
        map.force_insert(4, 18);
        map.force_insert(4, 19);
        const auto items = map.items(4);

        constexpr int check[]
        {
            16, 17, 18, 19
        };

        size_t count = 0;
        for (const auto& item : items)
        {
            D_ASSERT(item == check[count]);
            ++count;
        }

        D_ASSERT(count == std::size(check));
        D_ASSERT(map.erase(4) == std::size(check));
        check_contains(false, 4, 16);
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

            checker(int i) noexcept
                : i{ i }
            {}

            ~checker()
            {
                if (id)
                {
                    D_ASSERT(for_destroy().erase(id) == 1u);
                }
            }

            checker(const checker& right) noexcept
                : i{ right.i }
            {
                D_ASSERT(true);
            }

            checker& operator = (const checker&) noexcept
            {
                D_ASSERT(true);
                return *this;
            }

            checker(checker&& right) noexcept
                : i{ right.release_i() }
                , id{ right.release_id() }
            {}

            checker& operator = (checker&& right) noexcept
            {
                std::swap(i, right.i);
                std::swap(id, right.id);
                return *this;
            }

            bool operator < (const checker& right) const noexcept
            {
                return i < right.i;
            }

            bool operator == (const checker& right) const noexcept
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
                D_ASSERT(!id);
                id = unique_id();
                D_ASSERT(for_destroy().insert(id).second);
            }
        };

        using map_cc4 = small_flat_map<checker, checker, 4>;
        map_cc4 checker_map;

        auto insert = [&checker_map](int key, int value)
        {
            const auto ins = checker_map.insert(key, value);
            D_ASSERT(ins.second);
            ins.first->key.enable_check_destroy();
            ins.first->value.enable_check_destroy();
        };

        auto erase = [&checker_map](int key)
        {
            size_t test_count = 0u;
            map_cc4::const_iterator test_it = nullptr;
            for (auto it = checker_map.cbegin(); it < checker_map.cend(); ++it)
            {
                if (it->key == key)
                {
                    D_ASSERT(!test_count);
                    ++test_count;

                    D_ASSERT(!test_it);
                    test_it = it;
                }
            }
            D_ASSERT(test_count == 1u);
            D_ASSERT(test_it && test_it->key == key);

            const auto size = checker::for_destroy().size();
            const auto key_id = test_it->key.id;
            const auto value_id = test_it->value.id;

            D_ASSERT(size >= 2u);
            D_ASSERT(key_id && checker::for_destroy().contains(key_id));
            D_ASSERT(value_id && checker::for_destroy().contains(value_id));
            D_ASSERT(checker_map.erase(key) == 1u);
            D_ASSERT(checker::for_destroy().size() == size - 2u);
            D_ASSERT(!checker::for_destroy().contains(key_id));
            D_ASSERT(!checker::for_destroy().contains(value_id));
        };

        insert(1, 1);
        insert(4, 16);
        insert(2, 4);
        D_ASSERT(!checker_map.insert(2, 5).second);
        insert(6, 36);
        insert(10, 100);
        insert(7, 49);
        erase(7);
        erase(6);
        checker_map.erase(5);
        erase(4);
        checker_map.erase(3);
        erase(2);
        erase(1);
        erase(10);
    }
}
