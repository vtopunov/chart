#include <random>

#include <core/small_vector.h>
#include <core/utility.h>

namespace
{
    struct collector
    {
        using counter_t = uint16_t;
        static constexpr auto max_copies = std::numeric_limits<counter_t>::max();
        static constexpr size_t max_values{ 16384u };

        ptrdiff_t max_id{ 0 };
        counter_t all_copies[max_values]{};

        constexpr auto is_valid(ptrdiff_t id) const noexcept
        {
            return id >= 0 && id <= max_id;
        }

        constexpr void destroy(ptrdiff_t id) noexcept
        {
            D_ASSERT(is_valid(id));

            {
                auto& copies = all_copies[id];
                D_ASSERT(copies > 0);
                --copies;
            }
        }

        constexpr void create(ptrdiff_t id) noexcept
        {
            D_ASSERT(id >= 0 && id < max_values);
            max_id = std::max(max_id, id);

            {
                auto& copies = all_copies[id];
                D_ASSERT(!copies);
                ++copies;
            }
        }

        constexpr void dup(ptrdiff_t id) noexcept
        {
            D_ASSERT(is_valid(id));

            {
                auto& copies = all_copies[id];
                D_ASSERT(copies > 0 && copies < max_copies);
                ++copies;
            }
        }

        constexpr bool equal(const collector& right) const  noexcept
        {
            return right.max_id == max_id &&
                !memcmp(all_copies, right.all_copies, (max_id + 1u) * sizeof(counter_t));
        }

        constexpr ~collector() noexcept
        {
            for (ptrdiff_t i = 0; i <= max_id; ++i)
            {
                D_ASSERT(!all_copies[i]);
            }
        }
    };


    template<size_t num>
    inline static collector collect_instance_v{};

    template<size_t num>
    struct test_int
    {
        static constexpr ptrdiff_t invalid{ -1 };
        static constexpr collector* collect = &collect_instance_v<num>;

        ptrdiff_t value_;

        constexpr test_int(ptrdiff_t right) noexcept
            : value_{ right }
        {
            collect->create(right);
        }

        constexpr test_int(const test_int& right) noexcept
            : value_{ right.value_ }
        {
            collect->dup(right.value_);
        }

        constexpr test_int& operator = (const test_int& right) noexcept
        {
            if (this != std::addressof(right))
            {
                collect->destroy(value_);
                collect->dup(right.value_);
                value_ = right.value_;
            }

            return *this;
        }

        constexpr test_int(test_int&& right) noexcept
            : value_{ std::exchange(right.value_, invalid) }
        {
            collect->is_valid(value_);
        }

        constexpr test_int& operator = (test_int&& right) noexcept
        {
            swap(right);
            return *this;
        }

        constexpr void swap(test_int& right) noexcept
        {
            std::swap(value_, right.value_);
        }

        constexpr ~test_int() noexcept
        {
            if (value_ != invalid)
            {
                collect->destroy(value_);
            }
        }
    };

    template<size_t N, size_t M>
    constexpr auto operator == (const test_int<N>& l, const test_int<M>& r) noexcept
    {
        return l.value_ == r.value_;
    }

    template<size_t N, size_t M>
    constexpr auto operator != (const test_int<N>& l, const test_int<M>& r) noexcept
    {
        return l.value_ != r.value_;
    }

    template<class T, class TestT>
    struct test_destruction_t
    {
        constexpr void operator () () const noexcept
        {}
    };

    template<size_t N, size_t M>
    struct test_destruction_t<test_int<N>, test_int<M>>
    {
        constexpr void operator () () const noexcept
        {
            static_assert(N != M);
            constexpr auto collect = as_immutable(test_int<N>::collect);
            constexpr auto test_collect = as_immutable(test_int<M>::collect);
            static_assert(collect != test_collect);
            D_ASSERT(collect->equal(*test_collect));
        }
    };

    template<class T, class TestT, size_t N>
    struct vector_test
    {
        static constexpr auto static_size = N;
        static constexpr test_destruction_t<T, TestT> test_destruction{};

        using small_vector_type = small_vector<T, static_size>;
        using test_vector_type = std::vector<TestT>;

        small_vector_type small_v_;
        test_vector_type test_;

        vector_test() noexcept
        {
            test_state();
        }

        vector_test(const vector_test&) = default;

        vector_test& operator = (const vector_test&) = default;

        vector_test clone() const
        {
            return *this;
        }

        void test_state() const noexcept
        {
            const auto& c_small_v = std::as_const(small_v_);
            auto& mut_small_v = as_mutable(small_v_);

            test_capacity();

            const auto size = c_small_v.size();
            const auto data = c_small_v.data();
            const auto end_ptr = data + size;

            const auto cbegin = c_small_v.cbegin();
            const auto cend = c_small_v.cend();
            const auto beginc = c_small_v.begin();
            const auto endc = c_small_v.end();
            const auto begin = mut_small_v.begin();
            const auto end = mut_small_v.end();

            using test_pointer = T*;
            using test_const_pointer = const T*;
            using test_reference = T&;
            using test_const_reference = const T&;

            static_assert(std::is_same_v<std::remove_const_t<decltype(data)>, test_const_pointer>);
            static_assert(std::is_same_v<std::remove_const_t<decltype(cbegin)>, test_const_pointer>);
            static_assert(std::is_same_v<std::remove_const_t<decltype(cend)>, test_const_pointer>);
            static_assert(std::is_same_v<std::remove_const_t<decltype(beginc)>, test_const_pointer>);
            static_assert(std::is_same_v<std::remove_const_t<decltype(endc)>, test_const_pointer>);
            static_assert(std::is_same_v<std::remove_const_t<decltype(begin)>, test_pointer>);
            static_assert(std::is_same_v<std::remove_const_t<decltype(end)>, test_pointer>);

            D_ASSERT(cbegin == data);
            D_ASSERT(cend == end_ptr);
            D_ASSERT(beginc == data);
            D_ASSERT(endc == end_ptr);
            D_ASSERT(begin == data);
            D_ASSERT(end == end_ptr);

            static_assert(std::is_same_v<decltype(c_small_v.cfront()), test_const_reference>);
            static_assert(std::is_same_v<decltype(c_small_v.cback()), test_const_reference>);
            static_assert(std::is_same_v<decltype(c_small_v.front()), test_const_reference>);
            static_assert(std::is_same_v<decltype(c_small_v.back()), test_const_reference>);
            static_assert(std::is_same_v<decltype(mut_small_v.front()), test_reference>);
            static_assert(std::is_same_v<decltype(mut_small_v.back()), test_reference>);
            static_assert(std::is_same_v<decltype(c_small_v[0u]), test_const_reference>);
            static_assert(std::is_same_v<decltype(mut_small_v[0u]), test_reference>);

            if (size)
            {
                const auto back_ptr = std::prev(end_ptr);

                D_ASSERT(&c_small_v.cfront() == data);
                D_ASSERT(&c_small_v.cback() == back_ptr);
                D_ASSERT(&c_small_v.front() == data);
                D_ASSERT(&c_small_v.back() == back_ptr);
                D_ASSERT(&mut_small_v.front() == data);
                D_ASSERT(&mut_small_v.back() == back_ptr);
                D_ASSERT(&c_small_v[0u] == data);
                D_ASSERT(&c_small_v[size - 1u] == back_ptr);
                D_ASSERT(&mut_small_v[0u] == data);
                D_ASSERT(&mut_small_v[size - 1u] == back_ptr);
            }

            D_ASSERT(size == test_.size());

            static_assert(sizeof(T) == sizeof(TestT));
            D_ASSERT(!memcmp(data, test_.data(), size * sizeof(T)));

            test_destruction();
        }

        void test_capacity() const noexcept
        {
            const auto is_static = small_v_.is_static();
            D_ASSERT(small_v_.is_dynamic() == !is_static);

            const auto capacity = small_v_.capacity();

            if (is_static)
            {
                D_ASSERT(capacity == static_size);
            }
            else
            {
                D_ASSERT(!small_v_.data() || capacity > static_size);
            }

            D_ASSERT(capacity >= small_v_.size());
        }

        void test_is_static(bool is_static) const noexcept
        {
            D_ASSERT(small_v_.is_static() == is_static);
        }

        static ptrdiff_t generate_unique() noexcept
        {
            static ptrdiff_t unique{ -1 };
            return ++unique;
        }


        void fill(size_t count) noexcept
        {
            const auto size0 = small_v_.size();

            small_v_.reserve(small_v_.size() + count);
            test_.reserve(test_.size() + count);

            for (size_t i = 0; i < count; ++i)
            {
                const auto value = generate_unique();

                D_ASSERT(small_v_.try_emplace_back(value));
                test_.emplace_back(value);
            }

            test_state();
        }

        void test_emplace_back(size_t count) noexcept
        {
            const auto is_static0 = small_v_.is_static();
            const auto size0 = small_v_.size();

            test_.reserve(size0 + count);

            for (size_t i = 0; i < count; ++i)
            {
                const auto value = generate_unique();

                const auto& small_value_ref = *small_v_.try_emplace_back(value);
                const auto& big_value_ref = test_.emplace_back(value);

                D_ASSERT(small_value_ref == big_value_ref);

                const auto size = size0 + i + 1u;
                D_ASSERT(size == small_v_.size());
                test_is_static(is_static0 && size <= static_size);
                test_state();
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

        void test_copy_constuctor(const vector_test& v) noexcept
        {
            std::destroy_at(&small_v_);
            std::construct_at(&small_v_, v.small_v_);
            std::destroy_at(&test_);
            std::construct_at(&test_, v.test_);
            v.test_state();
            test_state();
        }

        void test_copy_assignment(const vector_test& v) noexcept
        {
            const auto data = small_v_.data();
            const auto capacity = small_v_.capacity();
            const auto v_size = v.small_v_.size();

            small_v_ = v.small_v_;
            test_ = v.test_;

            if (v_size > capacity)
            {
                D_ASSERT(v.small_v_.capacity() == v_size);
                D_ASSERT(v.small_v_.data() != data);
            }
            else
            {

                if (const auto test_capacity = std::max(static_size, v_size);
                    capacity > optimal_capacity_limit(test_capacity))
                {
                    D_ASSERT(small_v_.capacity() == test_capacity);
                    D_ASSERT(small_v_.data() != data);
                    test_is_static(v_size <= static_size);
                }
                else
                {
                    D_ASSERT(small_v_.capacity() == capacity);
                    D_ASSERT(small_v_.data() == data);
                }
            }

            v.test_state();
            test_state();
        }

        static void move_assignment(small_vector_type& left, small_vector_type& right) noexcept
        {
            left = std::move(right);
        }

        static void move_constuctor(small_vector_type& left, small_vector_type& right) noexcept
        {
            std::destroy_at(&left);
            std::construct_at(&left, std::move(right));
        }

        void test_move(vector_test& v, void (*move_op) (small_vector_type&, small_vector_type&)) noexcept
        {
            const auto size = small_v_.size();
            const auto data = small_v_.data();
            const auto is_static = small_v_.is_static();

            const auto v_size = v.small_v_.size();
            const auto v_data = v.small_v_.data();
            const auto v_is_static = v.small_v_.is_static();

            move_op(small_v_, v.small_v_);

            test_ = std::move(v.test_);

            if (is_static)
            {
                if (v_is_static)
                {
                    v.test_.clear();

                    D_ASSERT(v.small_v_.size() == 0u);
                    D_ASSERT(v.small_v_.data() == v_data);
                    D_ASSERT(v.small_v_.is_static());

                    D_ASSERT(small_v_.size() == v_size);
                    D_ASSERT(small_v_.data() == data);
                    D_ASSERT(small_v_.is_static());
                }
                else
                {
                    v.test_.clear();

                    D_ASSERT(v.small_v_.size() == 0u);
                    D_ASSERT(v.small_v_.data() != v_data);
                    D_ASSERT(v.small_v_.is_dynamic());

                    D_ASSERT(small_v_.size() == v_size);
                    D_ASSERT(small_v_.data() == v_data);
                    D_ASSERT(small_v_.is_dynamic());
                }
            }
            else
            {
                if (v_is_static)
                {
                    v.test_.clear();

                    D_ASSERT(v.small_v_.size() == 0u);
                    D_ASSERT(v.small_v_.data() == v_data);
                    D_ASSERT(v.small_v_.is_static());

                    D_ASSERT(small_v_.size() == v_size);
                    D_ASSERT(small_v_.data() != data);
                    D_ASSERT(small_v_.is_static());
                }
                else
                {
                    D_ASSERT(v.small_v_.size() == 0u);
                    D_ASSERT(v.small_v_.data() == data);
                    D_ASSERT(v.small_v_.is_dynamic());

                    D_ASSERT(small_v_.size() == v_size);
                    D_ASSERT(small_v_.data() == v_data);
                    D_ASSERT(small_v_.is_dynamic());
                }
            }

            v.test_state();
            test_state();
        }

        void test_move_constuctor(vector_test& v) noexcept
        {
            std::destroy_at(&small_v_);
            std::construct_at(&small_v_);

            test_move(v, move_constuctor);
        }

        void test_move_assignment(vector_test& v) noexcept
        {
            test_move(v, move_assignment);
        }

        void test_reserve(size_t new_capacity) noexcept
        {
            const auto data = small_v_.data();
            const auto capacity = small_v_.capacity();

            small_v_.reserve(new_capacity);

            if (new_capacity > static_size && new_capacity > capacity)
            {
                D_ASSERT(small_v_.data() != data);
                D_ASSERT(small_v_.capacity() == new_capacity);
            }
            else
            {
                D_ASSERT(small_v_.data() == data);
                D_ASSERT(small_v_.capacity() == capacity);
            }

            test_state();
        }

        void test_emplace(size_t index) noexcept
        {
            const auto value = generate_unique();

            const auto it = small_v_.try_emplace(small_v_.cbegin() + index, value);
            const auto test_it = test_.emplace(test_.cbegin() + index, value);

            D_ASSERT(*it == *test_it);

            const auto position = (small_v_.cbegin() - it);
            const auto test_position = (test_.cbegin() - test_it);

            D_ASSERT(position == test_position);

            test_state();
        }


        void test_switch_emplace(size_t index) noexcept
        {
            D_ASSERT(test_.size() == static_size);
            test_is_static(true);
            test_emplace(index);
            test_is_static(false);
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

        void test_shrink_to_fit() noexcept
        {
            const auto data = small_v_.data();
            const auto size = small_v_.size();
            const auto capacity = small_v_.capacity();
            const auto is_dynamic_capacity = capacity > static_size;
            const auto is_dynamic_size = size > static_size;

            test_is_static(!is_dynamic_capacity);

            small_v_.shrink_to_fit();

            test_is_static(!is_dynamic_size);

            if (is_dynamic_capacity && capacity > size)
            {
                D_ASSERT(small_v_.data() != data);

                if (is_dynamic_size)
                {
                    D_ASSERT(small_v_.capacity() == size);
                }
                else
                {
                    D_ASSERT(small_v_.capacity() == static_size);
                }
            }
            else
            {
                D_ASSERT(small_v_.data() == data);
                D_ASSERT(small_v_.capacity() == capacity);
            }

            test_state();
        }

        void test_shrink_erase(size_t index, size_t n) noexcept
        {
            test_erase(index, n);
            test_shrink_to_fit();
        }
    };

    template<class T, class TestT>
    struct tests
    {
        static constexpr size_t n_static{ 16u };

        using test_t = vector_test<T, TestT, n_static>;

        static void static_empty_init(test_t&) noexcept
        {};

        static void static_full_init(test_t& test) noexcept
        {
            test.fill(n_static);
        };

        static test_t static_full() noexcept
        {
            test_t test;
            static_full_init(test);
            return test;
        }

        static void dynamic_empty_init(test_t& test) noexcept
        {
            test.test_reserve(n_static + 1);
        };

        static void dynamic_min_init(test_t& test) noexcept
        {
            test.fill(n_static + 1);
        };

        static void dynamic_static_full_init(test_t& test) noexcept
        {
            dynamic_empty_init(test);
            static_full_init(test);
            test.test_is_static(false);
        };

        static void static_prefull_init(test_t& test) noexcept
        {
            test.fill(n_static - 1);
        };

        static void dynamic_ext_min_init(test_t& test) noexcept
        {
            test.fill(n_static + 2);
        };

        static constexpr auto capacity_growth0 = optimal_memory_growth(n_static + 1);
        static constexpr auto capacity_growth1 = optimal_memory_growth(capacity_growth0);
        static constexpr auto capacity_growth2 = optimal_memory_growth(capacity_growth1);
        static constexpr auto dynamic_big_size = capacity_growth2 + 1u;

        static void dynamic_big_init(test_t& test) noexcept
        {
            test.fill(dynamic_big_size);
        };

        static void dynamic_medium_init(test_t& test) noexcept
        {
            test.fill(capacity_growth1 + 1u);
        };

        static test_t dynamic_medium() noexcept
        {
            test_t test;
            dynamic_medium_init(test);
            return test;
        }

        static test_t dynamic_big() noexcept
        {
            test_t test;
            dynamic_big_init(test);
            return test;
        };

        static void dynamic_static_prefull_init(test_t& test) noexcept
        {
            dynamic_empty_init(test);
            static_prefull_init(test);
            test.test_is_static(false);
        };

        static void test_copy_constructor(test_t& left, test_t& right) noexcept
        {
            left.test_copy_constuctor(std::as_const(right));
        };

        static void test_move_constructor(test_t& left, test_t& right) noexcept
        {
            left.test_move_constuctor(right);
        };

        static void test_ñopy_assigment(test_t& left, test_t& right) noexcept
        {
            left.test_copy_assignment(std::as_const(right));
        };

        static void test_move_assignment(test_t& left, test_t& right) noexcept
        {
            left.test_move_assignment(right);
        };

        static void test_constructors_and_assignment_op() noexcept
        {
            using op1_t = void(*)(test_t&);
            using op2_t = void(*)(test_t&, test_t&);

            constexpr op1_t inits[]
            {
                static_empty_init,
                dynamic_empty_init,
                static_prefull_init,
                static_full_init,
                dynamic_static_prefull_init,
                dynamic_static_full_init,
                dynamic_min_init,
                dynamic_ext_min_init,
                dynamic_medium_init
            };

            for (const auto right_init : inits)
            {
                {
                    constexpr op2_t constructor_tests[]
                    {
                        test_copy_constructor,
                        test_move_constructor
                    };

                    for (const auto test : constructor_tests)
                    {
                        test_t left, right;
                        right_init(right);
                        test(left, right);
                    }
                }

                for (const auto left_init : inits)
                {
                    const auto stop = left_init == dynamic_medium_init;

                    {
                        constexpr op2_t assignment_op_tests[]
                        {
                            test_ñopy_assigment,
                            test_move_assignment
                        };

                        for (const auto test : assignment_op_tests)
                        {
                            test_t left, right;
                            left_init(left);
                            right_init(right);
                            test(left, right);
                        }
                    }
                }
            }
        }

        static void test_clear_shrink(size_t size) noexcept
        {
            test_t test;
            test.test_emplace_back(size);
            test.test_clear();
            test.test_shrink_to_fit();
        }

        static void test_pop_back_shrink(size_t size) noexcept
        {
            test_t test;
            test.test_emplace_back(size);
            test.test_pop_back();
            test.test_shrink_to_fit();
        }

        static void test_reserve(size_t size) noexcept
        {
            test_t test;
            test.test_reserve(size);
            const auto data = test.small_v_.data();
            test.test_emplace_back(size);
            D_ASSERT(data == test.small_v_.data());
        }

        static void test_switch_emplace() noexcept
        {
            const auto factory = static_full();

            for (size_t i = 0u; i < n_static; ++i)
            {
                factory.clone().test_switch_emplace(i);
            }
        }

        static void test_shrink_erase() noexcept
        {
            const auto test_shrink_erase = [] (const test_t& factory) noexcept
            {
                const auto size = factory.small_v_.size();

                for (size_t i = 0u; i < size; ++i)
                {
                    for (size_t count = 0u; count < (size - i); ++count)
                    {
                        factory.clone().test_shrink_erase(i, count);
                    }
                }
            };

            test_shrink_erase(static_full());
            test_shrink_erase(dynamic_medium());
        }

        static void test_switch_erase() noexcept
        {
            const auto factory = dynamic_big();

            for (size_t i = 0; i <= n_static; ++i)
            {
                test_t test{ factory };
                test.test_shrink_erase(i, test.small_v_.size() - n_static);
                test.test_is_static(true);
            }
        }

        static void test_emplace_random() noexcept
        {
            test_t test;

            std::mt19937_64 random_engine{ std::random_device{}() };

            for (size_t max_index = 0; max_index < dynamic_big_size; ++max_index)
            {
                const std::uniform_int_distribution<size_t> position_distribution(0u, max_index);
                const auto position = position_distribution(random_engine);
                test.test_emplace(position);
            }
        }

        static void test_erase_random() noexcept
        {
            test_t test;
            dynamic_big_init(test);

            std::mt19937_64 random_engine{ std::random_device{}() };
            const std::uniform_int_distribution<size_t> count_distribution{ 1u, 3u };

            while (const auto size = test.small_v_.size())
            {
                const auto count = std::min(count_distribution(random_engine), size);
                const std::uniform_int_distribution<size_t> position_distribution{ 0u, size - count };
                const auto position = position_distribution(random_engine);
                test.test_erase(position, count);
            }
        }

        static void test_all() noexcept
        {
            for (size_t i = n_static - 1; i < n_static + 3; ++i)
            {
                test_clear_shrink(i);
                test_pop_back_shrink(i);
                test_reserve(i);
            }

            test_constructors_and_assignment_op();
            test_emplace_random();
            test_switch_emplace();
            test_shrink_erase();
            test_switch_erase();
            test_erase_random();
        }
    };
}

void test_small_vector() noexcept
{
    tests<ptrdiff_t, ptrdiff_t>::test_all();
    tests<test_int<0>, test_int<1>>::test_all();
}