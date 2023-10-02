#include <core/tuple_algorithm.h>
#include <core/assert.h>

#include <algorithm>


namespace
{
    template<int>
    struct test_type {};

    template<int... I>
    using test_tuple = std::tuple<test_type<I>...>;
}


void test_tuple_algorithm() noexcept
{
    using test_tuple_0_5 = test_tuple<0, 1, 2, 3, 4>;
    using test_tuple_0_6 = test_tuple<0, 1, 2, 3, 4, 5>;

    using arithmetic_tuple1 = std::tuple<char>;
    using arithmetic_tuple2 = std::tuple<char, double>;
    using arithmetic_tuple6 = std::tuple<char, int16_t, char, double, char, float>;
    using arithmetic_tuple7 = std::tuple<char, int16_t, char, double, double, char, float>;

    static_assert(tuple_has_type_v<test_type<0>, test_tuple_0_5>);
    static_assert(tuple_has_type_v<test_type<4>, test_tuple_0_5>);
    static_assert(!tuple_has_type_v<test_type<5>, test_tuple_0_5>);
    static_assert(tuple_has_type_v<char, arithmetic_tuple6>);
    static_assert(!tuple_has_type_v<int32_t, arithmetic_tuple6>);
    static_assert(!tuple_has_type_v<char, std::tuple<>>);
    static_assert(!tuple_has_type_v<std::tuple<>, std::tuple<>>);
    static_assert(tuple_has_type_v<std::tuple<>, std::tuple<std::tuple<>>>);

    static_assert(std::is_same_v<tuple_swap_t<0, 0, test_tuple_0_5>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_swap_t<0, 1, test_tuple_0_5>, test_tuple<1, 0, 2, 3, 4>>);
    static_assert(std::is_same_v<tuple_swap_t<0, 2, test_tuple_0_5>, test_tuple<2, 1, 0, 3, 4>>);
    static_assert(std::is_same_v<tuple_swap_t<0, 3, test_tuple_0_5>, test_tuple<3, 1, 2, 0, 4>>);
    static_assert(std::is_same_v<tuple_swap_t<0, 4, test_tuple_0_5>, test_tuple<4, 1, 2, 3, 0>>);
    static_assert(std::is_same_v<tuple_swap_t<1, 0, test_tuple_0_5>, test_tuple<1, 0, 2, 3, 4>>);
    static_assert(std::is_same_v<tuple_swap_t<1, 1, test_tuple_0_5>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_swap_t<1, 2, test_tuple_0_5>, test_tuple<0, 2, 1, 3, 4>>);
    static_assert(std::is_same_v<tuple_swap_t<1, 3, test_tuple_0_5>, test_tuple<0, 3, 2, 1, 4>>);
    static_assert(std::is_same_v<tuple_swap_t<1, 4, test_tuple_0_5>, test_tuple<0, 4, 2, 3, 1>>);
    static_assert(std::is_same_v<tuple_swap_t<4, 0, test_tuple_0_5>, test_tuple<4, 1, 2, 3, 0>>);
    static_assert(std::is_same_v<tuple_swap_t<4, 1, test_tuple_0_5>, test_tuple<0, 4, 2, 3, 1>>);
    static_assert(std::is_same_v<tuple_swap_t<4, 2, test_tuple_0_5>, test_tuple<0, 1, 4, 3, 2>>);
    static_assert(std::is_same_v<tuple_swap_t<4, 3, test_tuple_0_5>, test_tuple<0, 1, 2, 4, 3>>);
    static_assert(std::is_same_v<tuple_swap_t<4, 4, test_tuple_0_5>, test_tuple<0, 1, 2, 3, 4>>);

    static_assert(std::is_same_v<tuple_pop_front_t<arithmetic_tuple1>, std::tuple<>>);
    static_assert(std::is_same_v<tuple_pop_front_t<arithmetic_tuple2>, std::tuple<double>>);
    static_assert(std::is_same_v<tuple_pop_front_t<test_tuple_0_5>, test_tuple<1, 2, 3, 4>>);
    static_assert(std::is_same_v<tuple_pop_front_t<test_tuple<333, 666, 777>>, test_tuple<666, 777>>);

    static_assert(std::is_same_v<tuple_push_front_t<char, std::tuple<>>, std::tuple<char>>);
    static_assert(std::is_same_v<tuple_push_front_t<float, std::tuple<char>>, std::tuple<float, char>>);
    static_assert(std::is_same_v<tuple_push_front_t<test_type<777>, test_tuple<666>>, test_tuple<777, 666>>);
    static_assert(std::is_same_v<tuple_push_front_t<test_type<5>, test_tuple<6, 7>>, test_tuple<5, 6, 7>>);

    static_assert(std::is_same_v<tuple_push_back_t<std::tuple<>, char>, std::tuple<char>>);
    static_assert(std::is_same_v<tuple_push_back_t<std::tuple<char>, float>, std::tuple<char, float>>);
    static_assert(std::is_same_v<tuple_push_back_t<test_tuple<666>, test_type<777>>, test_tuple<666, 777>>);
    static_assert(std::is_same_v<tuple_push_back_t<test_tuple<6, 7>, test_type<8>>, test_tuple<6, 7, 8>>);

    static_assert(std::is_same_v<tuple_cat_t<test_tuple<>>, test_tuple<>>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple_0_5>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<>, test_tuple<>>, test_tuple<>>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<>, test_tuple<0, 1, 2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0>, test_tuple<1, 2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1>, test_tuple<2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1, 2>, test_tuple<3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1, 2, 3>, test_tuple<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1, 2, 3, 4>, test_tuple<>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<>, test_tuple<>, test_tuple<>>, test_tuple<>>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0>, test_tuple<1>, test_tuple<2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0>, test_tuple<1, 2, 3>, test_tuple<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1, 2>, test_tuple<3>, test_tuple<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1>, test_tuple<2, 3>, test_tuple<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1>, test_tuple<2>, test_tuple<3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0>, test_tuple<1, 2>, test_tuple<3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1>, test_tuple<2, 3>, test_tuple<4, 5>>, test_tuple_0_6>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1>, test_tuple<2, 3, 4, 5>, test_tuple<>>, test_tuple_0_6>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1>, test_tuple<>, test_tuple<2, 3, 4, 5>>, test_tuple_0_6>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<>, test_tuple<0, 1>, test_tuple<2, 3, 4, 5>>, test_tuple_0_6>);

    using index_sequence3 = std::make_index_sequence<3>;
    static_assert(std::is_same_v<index_sequence3, std::index_sequence<0, 1, 2>>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<index_sequence3>, std::index_sequence<1, 2>>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<std::make_index_sequence<2>>, std::index_sequence<1>>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<std::make_index_sequence<1>>, std::index_sequence<>>);
    static_assert(std::is_same_v<index_sequence_push_front_t<33, index_sequence3>, std::index_sequence<33, 0, 1, 2>>);

    static_assert(0u == min_tuple_index_element_v<less_sizeof, arithmetic_tuple1>);
    static_assert(0u == min_tuple_index_element_v<less_eq_sizeof, arithmetic_tuple1>);
    static_assert(0u == min_tuple_index_element_v<greater_sizeof, arithmetic_tuple1>);
    static_assert(0u == min_tuple_index_element_v<greater_eq_sizeof, arithmetic_tuple1>);
    static_assert(0u == min_tuple_index_element_v<less_sizeof, arithmetic_tuple2>);
    static_assert(0u == min_tuple_index_element_v<less_eq_sizeof, arithmetic_tuple2>);
    static_assert(1u == min_tuple_index_element_v<greater_sizeof, arithmetic_tuple2>);
    static_assert(1u == min_tuple_index_element_v<greater_eq_sizeof, arithmetic_tuple2>);
    static_assert(4u == min_tuple_index_element_v<less_sizeof, arithmetic_tuple6>);
    static_assert(0u == min_tuple_index_element_v<less_eq_sizeof, arithmetic_tuple6>);
    static_assert(3u == min_tuple_index_element_v<greater_sizeof, arithmetic_tuple6>);
    static_assert(3u == min_tuple_index_element_v<greater_eq_sizeof, arithmetic_tuple6>);
    static_assert(5u == min_tuple_index_element_v<less_sizeof, arithmetic_tuple7>);
    static_assert(0u == min_tuple_index_element_v<less_eq_sizeof, arithmetic_tuple7>);
    static_assert(4u == min_tuple_index_element_v<greater_sizeof, arithmetic_tuple7>);
    static_assert(3u == min_tuple_index_element_v<greater_eq_sizeof, arithmetic_tuple7>);

    using small2big_arithmetic_tuple6 = std::tuple<char, char, char, int16_t, float, double>;
    using big2small_arithmetic_tuple6 = std::tuple<double, float, int16_t, char, char, char>;

    using small2big_arithmetic_tuple7 = std::tuple<char, char, char, int16_t, float, double, double>;
    using big2small_arithmetic_tuple7 = std::tuple<double, double, float, int16_t, char, char, char>;

    static_assert(std::is_same_v<sort_tuple_t<greater_eq_sizeof, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<sort_tuple_t<less_sizeof, arithmetic_tuple6>, small2big_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_tuple_t<less_eq_sizeof, arithmetic_tuple6>, small2big_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_tuple_t<greater_sizeof, arithmetic_tuple6>, big2small_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_tuple_t<greater_eq_sizeof, arithmetic_tuple6>, big2small_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_tuple_t<less_sizeof, arithmetic_tuple7>, small2big_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_tuple_t<less_eq_sizeof, arithmetic_tuple7>, small2big_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_tuple_t<greater_sizeof, arithmetic_tuple7>, big2small_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_tuple_t<greater_eq_sizeof, arithmetic_tuple7>, big2small_arithmetic_tuple7>);

    static_assert(!std::is_same_v<char32_t, int32_t>);
    static_assert(!std::is_same_v<char32_t, uint32_t>);
    static_assert(4u == std::max(std::max(std::max(sizeof(char32_t), sizeof(int32_t)), sizeof(uint32_t)), sizeof(float)));
    using arithmetic_eq_sizeof_tuple = std::tuple<char32_t, int32_t, uint32_t, float>;

    static_assert(std::is_same_v<sort_tuple_t<less_eq_sizeof, arithmetic_eq_sizeof_tuple>, arithmetic_eq_sizeof_tuple>);
    static_assert(std::is_same_v<sort_tuple_t<greater_eq_sizeof, arithmetic_eq_sizeof_tuple>, arithmetic_eq_sizeof_tuple>);

#ifdef _MSC_VER
    static_assert(sizeof(tuple_sizeof_optimization_t<arithmetic_tuple7>) < sizeof(arithmetic_tuple7));
#else
    static_assert(sizeof(tuple_sizeof_optimization_t<arithmetic_tuple7>) <= sizeof(arithmetic_tuple7));
#endif    

    static_assert(std::is_same_v<tuple_unique_push_back_t<std::tuple<>, char>, std::tuple<char>>);
    static_assert(std::is_same_v<tuple_unique_push_back_t<std::tuple<char>, char>, std::tuple<char>>);
    static_assert(std::is_same_v<tuple_unique_push_back_t<std::tuple<char>, double>, std::tuple<char, double>>);

    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<std::tuple<>, std::tuple<char>>, std::tuple<char>>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<std::tuple<char>, std::tuple<>>, std::tuple<char>>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<std::tuple<char>, std::tuple<char>>, std::tuple<char>>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<std::tuple<char>, std::tuple<char, int>>, std::tuple<char, int>>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<test_tuple<1, 2, 5>, test_tuple_0_6>, test_tuple<1, 2, 5, 0, 3, 4>>);


    {
        using test_unique_t = test_tuple<0, 1, 2>;

        static_assert(std::is_same_v<unique_tuple_t<std::tuple<>>, std::tuple<>>);
        static_assert(std::is_same_v<unique_tuple_t<test_unique_t>, test_unique_t>);
        static_assert(std::is_same_v<unique_tuple_t<test_tuple<0, 1, 1, 2>>, test_unique_t>);
        static_assert(std::is_same_v<unique_tuple_t<test_tuple<0, 1, 1, 2, 2>>, test_unique_t>);
        static_assert(std::is_same_v<unique_tuple_t<test_tuple<0, 1, 0, 2>>, test_unique_t>);
        static_assert(std::is_same_v<unique_tuple_t<test_tuple<0, 1, 0, 2, 1>>, test_unique_t>);
        static_assert(std::is_same_v<unique_tuple_t<test_tuple<0, 1, 0, 2, 1, 2>>, test_unique_t>);
        static_assert(std::is_same_v<unique_tuple_t<test_tuple<0, 0, 1, 2>>, test_unique_t>);
    }

    D_ASSERT(!errno);
}
