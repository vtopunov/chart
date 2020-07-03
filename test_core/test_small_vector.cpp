#include <array>
#include <set>
#include <random>
#include <functional>

#include <core/small_vector.h>
#include <core/assert.h>

template<class T, size_t N>
struct vector_test
{
    static constexpr auto small_size = N;
    using small_vector_type = small_vector<T, small_size>;

    small_vector_type small_v_;
    std::vector<T> test_;

    void test_push_back(const T& value) noexcept
    {
        small_v_.push_back(value);
        test_.push_back(value);
        test_state();
    }

    void test_push_back_loop(T from, T to) noexcept
    {
        for (; from <= to; ++from)
        {
            test_push_back(from);
        }
    }

    void test_clear() noexcept
    {
        small_v_.clear();
        test_.clear();
        test_state();
    }

    void test_pop_back() noexcept
    {
        small_v_.pop_back();
        test_.pop_back();
        test_state();
    }

    void test_copy_constuctor() const noexcept
    {
        const auto small_v = small_v_;
        test_state(small_v);
        test_state();
    }

    void test_move_constuctor() noexcept
    {
        small_vector_type small_v{ std::move(small_v_) };
        test_state(small_v);

#pragma warning(push)
#pragma warning(disable : 26800) // use of a moved from object
        std::destroy_at(&small_v_);
#pragma warning(pop)

        new ( &small_v_ ) small_vector_type{ std::move(small_v) };

        test_state();
    }

    void test_copy_assignment() const noexcept
    {
        small_vector_type small_v;
        small_v = small_v_;
        test_state(small_v);
        test_state();
    }

    void test_move_assignment() noexcept
    {
        small_vector_type small_v;
        small_v = std::move(small_v_);
        test_state(small_v);
        small_v_ = std::move(small_v);
        test_state();
    }

    void test_state(const small_vector_type& small_v) const noexcept
    {
        if ( small_v.is_small() )
        {
            D_ASSERT(small_v.capacity() == small_size);
        }
        else
        {
            D_ASSERT(small_v.capacity() > small_size);
        }

        D_ASSERT(small_v.size() <= small_v.capacity());
        D_ASSERT(small_v.cbegin() == small_v.data());
        D_ASSERT(small_v.cend() == small_v.data() + small_v.size());
        D_ASSERT(small_v.size() == test_.size());
        D_ASSERT(std::equal(test_.cbegin(), test_.cend(), small_v.data()));
    }

    void test_state() const noexcept
    {
        test_state(small_v_);
    }

    void test_copy_move() noexcept
    {
        test_copy_constuctor();
        test_copy_assignment();
        test_move_constuctor();
        test_move_assignment();
    }

    void test_shrink_to_fit() noexcept
    {
        D_ASSERT(small_v_.capacity() > test_.size());
        test_is_small(false);
        small_v_.shrink_to_fit();
        test_is_small(test_.size() <= small_size);
        test_state();
    }

    void test_is_small(bool is_small) const noexcept
    {
        D_ASSERT(small_v_.is_small() == is_small);
    }

    void test_reserve(size_t size) noexcept
    {
        D_ASSERT(small_v_.capacity() < size);
        small_v_.reserve(size);
        D_ASSERT(small_v_.capacity() >= size);
        test_state();
    }

    void test_insert(size_t index, const T& value) noexcept
    {
        const auto result = small_v_.insert(small_v_.cbegin() + index, value);
        D_ASSERT(*result == value);

        test_.insert(test_.cbegin() + index, value);
        test_state();
    }


    void test_switch_insert(size_t index, const T& value) noexcept
    {
        D_ASSERT(test_.size() == small_size);
        test_is_small(true);
        test_insert(index, value);
        test_is_small(false);
    }

    void test_erase(size_t index, size_t n) noexcept
    {
        const auto last_index = index + n;

        {
            const auto v_begin = small_v_.cbegin();
            small_v_.erase(v_begin + index, v_begin + last_index);
        }

        {
            const auto test_begin = test_.cbegin();
            test_.erase(test_begin + index, test_begin + last_index);
        }

        test_state();
    }

    void test_shrink_erase(size_t index, size_t n) noexcept
    {
        test_erase(index, n);
        small_v_.shrink_to_fit();
        test_is_small(test_.size() <= small_size);
        test_state();
    }

    vector_test clone() const
    {
        return *this;
    }

    size_t size() const noexcept
    {
        return test_.size();
    }
};


struct collect
{
    std::set<int> init_;
    std::set<int> destroy_;

    void destroy(int id) noexcept
    {
        D_ASSERT(init_.erase(id) == 1u);
        D_ASSERT(destroy_.insert(id).second);
    }

    void init(int id) noexcept
    {
        D_ASSERT(init_.insert(id).second);
        D_ASSERT(!destroy_.contains(id));
    }

    bool equal(const collect& right) const
    {
        return init_.size() == right.init_.size() 
            && std::equal(init_.cbegin(), init_.cend(), right.init_.cbegin())
            && destroy_.size() == right.destroy_.size()
            && std::equal(destroy_.cbegin(), destroy_.cend(), right.destroy_.cbegin());
    }
};

struct safe_int
{
    int value_{ 0 };
    std::function<void(int)> dtor_task_;

    safe_int() noexcept = default;

    safe_int(int value) noexcept
        : value_{ value }
    {}

    safe_int(const safe_int&) = delete;

    safe_int& operator = (const safe_int&) = delete;

    safe_int(safe_int&& value) noexcept
        : value_{ std::exchange(value.value_, 0) }
        , dtor_task_{ std::exchange(value.dtor_task_, nullptr) }
    {}

    safe_int& operator = (safe_int&& value) noexcept
    {
        swap(value);
        return *this;
    }

    void swap(safe_int& value) noexcept
    {
        std::swap(value_, value.value_);
        std::swap(dtor_task_, value.dtor_task_);
    }

    ~safe_int() noexcept
    {
        if ( dtor_task_ )
        {
            dtor_task_(value_);
        }
    }

    bool operator == (const safe_int& right) const noexcept
    {
        return value_ == right.value_;
    }

    bool operator != (const safe_int& right) const noexcept
    {
        return value_ != right.value_;
    }
};

struct vector_test_dtor
{
    vector_test<safe_int, 32> test_;
    collect small_v_collect_;
    collect test_collect_;

    void test_state()
    {
        test_.test_state();
        D_ASSERT(small_v_collect_.equal(test_collect_));
    }

    void test_insert()
    {
        for ( size_t i = 0; i < 3 * test_.small_size; ++i )
        {
            const auto value = narrow_cast<int>( i + 1u );
            test_.small_v_.push_back(value);
            test_.test_.push_back(value);
        }

        for ( auto& value : test_.small_v_ )
        {
            small_v_collect_.init(value.value_);

            value.dtor_task_ = [this] (int value) noexcept
            {
                small_v_collect_.destroy(value);
            };
        }

        for ( auto& value : test_.test_ )
        {
            test_collect_.init(value.value_);

            value.dtor_task_ = [this] (int value) noexcept
            {
                test_collect_.destroy(value);
            };
        }

        test_state();
    }

    void test_erase()
    {
        std::random_device rd;  
        std::mt19937 gen(rd()); 
        std::uniform_int_distribution<size_t> gen_n(1u, 3u);

        while ( const auto size = test_.size() )
        {
            const auto n = std::min(gen_n(gen), size);
            std::uniform_int_distribution<size_t> gen_pos(0u, size - n);
            test_.test_erase(gen_pos(gen), n);
            test_state();
        }
    }

};

void test_small_vector() noexcept
{
    {
        vector_test<int, 4> test4;
        test4.test_state();
        test4.test_copy_move();

        test4.test_push_back_loop(1, 3);
        test4.test_clear();
        test4.test_copy_move();
        test4.test_push_back_loop(1, 3);
        test4.test_is_small(true);
        test4.test_copy_move();

        test4.test_push_back(4);
        test4.test_is_small(true);
        test4.test_push_back(5);
        test4.test_is_small(false);
        test4.test_copy_move();
        test4.test_clear();
        test4.test_copy_move();
        test4.test_push_back_loop(1, 5);
        test4.test_copy_move();
        test4.test_is_small(false);

        test4.test_is_small(false);
        test4.test_shrink_to_fit();
        test4.test_is_small(false);

        test4.test_pop_back();
        test4.test_pop_back();

        test4.test_is_small(false);
        test4.test_shrink_to_fit();
        test4.test_is_small(true);

        test4.test_reserve(5);
        test4.test_is_small(false);
        test4.test_shrink_to_fit();
        test4.test_is_small(true);

        test4.test_reserve(100);
        test4.test_is_small(false);
        test4.test_push_back(4);
        test4.test_push_back(5);
        test4.test_shrink_to_fit();
        test4.test_is_small(false);
        test4.test_reserve(6);
        test4.test_is_small(false);
    }

    {
        vector_test<int, 9> test9;
        test9.test_insert(0, 1);
        test9.test_insert(0, 2);
        test9.test_insert(0, 3);
        test9.test_insert(1, 4);
        test9.test_insert(2, 5);
        test9.test_insert(1/*3*/, 6);
        test9.test_insert(test9.size() - 2, 7);
        test9.test_insert(test9.size() - 1, 8);
        test9.test_insert(test9.size(), 9);
        test9.test_copy_move();

        {
            constexpr int value{ 0x12345678 };
            const auto size = test9.size();
            for ( size_t i = 0u; i < size; ++i )
            {
                test9.clone().test_switch_insert(i, value);
            }
        }

        test9.test_insert(1, 17);
        test9.test_insert(2, 18);
        test9.test_insert(3, 19);
        test9.test_insert(test9.size() - 2, 20);
        test9.test_insert(test9.size() - 1, 21);
        test9.test_insert(test9.size(), 22);
        test9.test_copy_move();

        auto test_erase = [&test9] () noexcept
        {
            const auto size = test9.size();
            for ( size_t i = 0u; i < size; ++i )
            {
                for ( size_t n = 0u; n < ( size - i ); ++n )
                {
                    test9.clone().test_shrink_erase(i, n);
                }
            }
        };

        test_erase();

        test9.test_is_small(false);
        test9.test_erase(test9.small_size, test9.size() - test9.small_size);
        test9.test_shrink_to_fit();
        test9.test_is_small(true);
        test9.test_copy_move();

        test_erase();
    }

    {
        vector_test_dtor test;
        test.test_insert();
        test.test_erase();
    }
}