#include <core/types_algorithm.h>
#include <core/fwd.h>

#include <algorithm>
#include <tuple>

namespace
{
    template<int>
    struct test_type {};

    template<int... I>
    using test_tuple = std::tuple<test_type<I>...>;

    template<int... call_ids>
    struct test_call_type
    {
        constexpr void operator () (test_type<call_ids>...) const noexcept
        {}
    };

    template<int...>
    struct test_self_call
    {
        template<int... Ids>
        constexpr void operator () (const test_self_call<Ids...>&) const noexcept
        {}
    };
}


void test_tuple_algorithm() noexcept
{
    using test_tuple_0_4 = test_tuple<0, 1, 2, 3>;
    using test_tuple_0_5 = test_tuple<0, 1, 2, 3, 4>;
    using test_tuple_0_6 = test_tuple<0, 1, 2, 3, 4, 5>;
    using test_tuple_0_7 = test_tuple<0, 1, 2, 3, 4, 5, 6>;
    using test_tuple_0_8 = test_tuple<0, 1, 2, 3, 4, 5, 6, 7>;
    using test_tuple_0_9 = test_tuple<0, 1, 2, 3, 4, 5, 6, 7, 8>;
    using seq0 = std::make_index_sequence<0u>;
    using seq1 = std::make_index_sequence<1u>;
    using seq2 = std::make_index_sequence<2u>;
    using seq12 = std::index_sequence<1u>;
    using seq3 = std::make_index_sequence<3u>;
    using seq13 = std::index_sequence<1u, 2u>;
    using seq31 = std::index_sequence<2u, 1u>;
    using seq4 = std::make_index_sequence<4u>;
    using seq14 = std::index_sequence<1u, 2u, 3u>;


    using tuple1_c = std::tuple<char>;
    using tuple1_d = std::tuple<double>;
    using tuple2_cc = std::tuple<char, char>;
    using tuple2_cd = std::tuple<char, double>;
    using tuple6_ci16cdcf = std::tuple<char, int16_t, char, double, char, float>;
    using tuple7_ci16cddcf = std::tuple<char, int16_t, char, double, double, char, float>;

    using unique_resource10 = unique_resource<test_self_call<1>, test_self_call<0>>;
    using unique_resource01 = unique_resource<test_self_call<0>, test_self_call<1>>;

    static_assert(0u == types_size_v<char>);
    static_assert(0u == types_size_v<std::tuple<>>);
    static_assert(1u == types_size_v<tuple1_c>);
    static_assert(2u == types_size_v<tuple2_cc>);
    static_assert(6u == types_size_v<tuple6_ci16cdcf>);
    static_assert(6u == types_size_v<tuple6_ci16cdcf>);
    static_assert(std::is_same_v<dummy, transform_types_t<std::add_const_t, dummy>>);
    static_assert(std::is_same_v<std::tuple<>, transform_types_t<std::add_const_t, std::tuple<>>>);
    static_assert(std::is_same_v<std::tuple<const char>, transform_types_t<std::add_const_t, tuple1_c>>);

    static_assert(std::is_same_v<std::tuple<const double, const char>, transform_types_t<std::add_const_t, std::tuple<double, char>>>);
    static_assert(std::is_same_v<std::make_index_sequence<6u>, make_types_index_sequence<tuple6_ci16cdcf>>);

    static_assert(types_has_type_v<test_type<0>, test_tuple_0_5>);
    static_assert(types_has_type_v<test_type<4>, test_tuple_0_5>);
    static_assert(!types_has_type_v<test_type<5>, test_tuple_0_5>);
    static_assert(types_has_type_v<char, tuple6_ci16cdcf>);
    static_assert(!types_has_type_v<int32_t, tuple6_ci16cdcf>);
    static_assert(!types_has_type_v<char, std::tuple<>>);
    static_assert(!types_has_type_v<std::tuple<>, std::tuple<>>);
    static_assert(types_has_type_v<std::tuple<>, std::tuple<std::tuple<>>>);

    static_assert(types_has_call_v<std::tuple<test_call_type<0>>, test_type<0>>);
    static_assert(!types_has_call_v<std::tuple<test_call_type<0>>, test_type<1>>);
    static_assert(types_has_call_v<std::tuple<test_call_type<0>, test_call_type<1>>, test_type<1>>);
    static_assert(types_has_call_v<std::tuple<test_call_type<0>, test_call_type<0, 1>>, test_type<0>, test_type<1>>);

    static_assert(std::is_same_v<char, types_element_t<0u, char>>);
    static_assert(std::is_same_v<char, types_element_t<0u, char, int>>);
    static_assert(std::is_same_v<int, types_element_t<1u, char, int>>);
    static_assert(std::is_same_v<double, types_element_t<2u, char, int, double>>);
    static_assert(std::is_same_v<int, types_element_t<1u, char, int, double>>);
    static_assert(std::is_same_v<char, types_element_t<0u, char, int, double>>);

    static_assert(std::is_same_v<types_swap_t<0, 0, test_tuple_0_5>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_swap_t<0, 1, test_tuple_0_5>, test_tuple<1, 0, 2, 3, 4>>);
    static_assert(std::is_same_v<types_swap_t<0, 2, test_tuple_0_5>, test_tuple<2, 1, 0, 3, 4>>);
    static_assert(std::is_same_v<types_swap_t<0, 3, test_tuple_0_5>, test_tuple<3, 1, 2, 0, 4>>);
    static_assert(std::is_same_v<types_swap_t<0, 4, test_tuple_0_5>, test_tuple<4, 1, 2, 3, 0>>);
    static_assert(std::is_same_v<types_swap_t<1, 0, test_tuple_0_5>, test_tuple<1, 0, 2, 3, 4>>);
    static_assert(std::is_same_v<types_swap_t<1, 1, test_tuple_0_5>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_swap_t<1, 2, test_tuple_0_5>, test_tuple<0, 2, 1, 3, 4>>);
    static_assert(std::is_same_v<types_swap_t<1, 3, test_tuple_0_5>, test_tuple<0, 3, 2, 1, 4>>);
    static_assert(std::is_same_v<types_swap_t<1, 4, test_tuple_0_5>, test_tuple<0, 4, 2, 3, 1>>);
    static_assert(std::is_same_v<types_swap_t<4, 0, test_tuple_0_5>, test_tuple<4, 1, 2, 3, 0>>);
    static_assert(std::is_same_v<types_swap_t<4, 1, test_tuple_0_5>, test_tuple<0, 4, 2, 3, 1>>);
    static_assert(std::is_same_v<types_swap_t<4, 2, test_tuple_0_5>, test_tuple<0, 1, 4, 3, 2>>);
    static_assert(std::is_same_v<types_swap_t<4, 3, test_tuple_0_5>, test_tuple<0, 1, 2, 4, 3>>);
    static_assert(std::is_same_v<types_swap_t<4, 4, test_tuple_0_5>, test_tuple<0, 1, 2, 3, 4>>);
    static_assert(std::is_same_v<types_swap_t<0u, 1u, unique_resource01>, unique_resource10>);

    static_assert(std::is_same_v<reorder_types_t<test_tuple_0_5, seq4>, test_tuple_0_4>);
    static_assert(std::is_same_v<reorder_types_t<test_tuple_0_5, seq14>, test_tuple<1, 2, 3>>);
    static_assert(std::is_same_v<reorder_types_t<test_tuple_0_5, seq31>, test_tuple<2, 1>>);
    static_assert(std::is_same_v<reorder_types_t<seq4, seq14>, seq14>);
    static_assert(std::is_same_v<reorder_types_t<seq4, seq13>, seq13>);
    static_assert(std::is_same_v<reorder_types_t<seq4, seq31>, seq31>);

    static_assert(std::is_same_v<types_if_t<std::is_integral, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<types_if_t<std::is_integral, tuple2_cd>, tuple1_c>);
    static_assert(std::is_same_v<types_if_t<std::is_floating_point, tuple2_cd>, tuple1_d>);
    static_assert(std::is_same_v<types_if_t<std::is_arithmetic, tuple2_cd>, tuple2_cd>);
    static_assert(std::is_same_v<types_if_t<std::is_class, tuple2_cd>, std::tuple<>>);
    static_assert(std::is_same_v<types_if_t<std::is_integral, tuple2_cd>, tuple1_c>);
    static_assert(std::is_same_v<types_if_t<std::is_floating_point, tuple6_ci16cdcf>, std::tuple<double, float>>);
    static_assert(std::is_same_v<types_if_t<std::is_floating_point, tuple7_ci16cddcf>, std::tuple<double, double, float>>);
    static_assert(std::is_same_v<types_if_t<std::is_class, tuple7_ci16cddcf>, std::tuple<>>);

    static_assert(std::is_same_v<types_split_if_left_t<std::is_integral, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<types_split_if_right_t<std::is_integral, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<types_split_if_left_t<std::is_integral, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<types_split_if_right_t<std::is_integral, tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<types_split_if_left_t<std::is_integral, tuple2_cd>, tuple1_c>);
    static_assert(std::is_same_v<types_split_if_right_t<std::is_integral, tuple2_cd>, tuple1_d>);
    static_assert(std::is_same_v<types_split_if_left_t<std::is_floating_point, tuple2_cd>, tuple2_cd>);
    static_assert(std::is_same_v<types_split_if_right_t<std::is_floating_point, tuple2_cd>, std::tuple<>>);
    static_assert(std::is_same_v<types_split_if_left_t<std::is_arithmetic, tuple2_cd>, tuple1_c>);
    static_assert(std::is_same_v<types_split_if_right_t<std::is_arithmetic, tuple2_cd>, tuple1_d>);
    static_assert(std::is_same_v<types_split_if_left_t<std::is_class, tuple2_cd>, std::tuple<>>);
    static_assert(std::is_same_v<types_split_if_right_t<std::is_class, tuple2_cd>, std::tuple<>>);
    static_assert(std::is_same_v<types_split_if_left_t<std::is_floating_point, tuple6_ci16cdcf>, std::tuple<char, int16_t, char, double>>);
    static_assert(std::is_same_v<types_split_if_right_t<std::is_floating_point, tuple6_ci16cdcf>, std::tuple<char, float>>);
    static_assert(std::is_same_v<types_split_if_left_t<std::is_floating_point, tuple7_ci16cddcf>, std::tuple<char, int16_t, char, double>>);
    static_assert(std::is_same_v<types_split_if_right_t<std::is_floating_point, tuple7_ci16cddcf>, std::tuple<double, char, float>>);

    static_assert(std::is_same_v<types_if_t<std::is_floating_point, tuple7_ci16cddcf>, std::tuple<double, double, float>>);

    static_assert(std::is_same_v<types_if_t<std::is_integral, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<types_if_t<std::is_class, tuple7_ci16cddcf>, std::tuple<>>);

    static_assert(std::is_same_v<types_pop_front_t<tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<types_pop_front_t<tuple2_cd>, tuple1_d>);
    static_assert(std::is_same_v<types_pop_front_t<test_tuple_0_5>, test_tuple<1, 2, 3, 4>>);
    static_assert(std::is_same_v<types_pop_front_t<test_tuple<333, 666, 777>>, test_tuple<666, 777>>);

    static_assert(std::is_same_v<types_pop_front_t<seq1>, seq0>);
    static_assert(std::is_same_v<types_pop_front_t<seq2>, seq12>);
    static_assert(std::is_same_v<types_pop_front_t<seq3>, seq13>);
    static_assert(std::is_same_v<types_pop_front_t<seq4>, seq14>);
    static_assert(0u == types_pop_front<seq1>::value.second);
    static_assert(1u == types_pop_front<seq12>::value.second);
    static_assert(1u == types_pop_front<seq13>::value.second);
    static_assert(2u == types_pop_front<seq31>::value.second);
    static_assert(std::is_same_v<index_sequence_push_front_t<33, seq3>, std::index_sequence<33, 0, 1, 2>>);

    static_assert(std::is_same_v<types_pop_back_t<tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<types_pop_back_t<tuple2_cd>, std::tuple<char>>);
    static_assert(std::is_same_v<types_pop_back_t<test_tuple_0_5>, test_tuple_0_4>);
    static_assert(std::is_same_v<types_pop_back_t<test_tuple<333, 666, 777>>, test_tuple<333, 666>>);

    static_assert(std::is_same_v<types_pop_back_t<seq1>, seq0>);
    static_assert(std::is_same_v<types_pop_back_t<seq2>, seq1>);
    static_assert(std::is_same_v<types_pop_back_t<seq3>, seq2>);
    static_assert(std::is_same_v<types_pop_back_t<seq4>, seq3>);
    static_assert(0u == types_pop_back<seq1>::value.second);
    static_assert(1u == types_pop_back<seq12>::value.second);
    static_assert(2u == types_pop_back<seq13>::value.second);
    static_assert(1u == types_pop_back<seq31>::value.second);
    static_assert(std::is_same_v<index_sequence_push_front_t<33, seq3>, std::index_sequence<33, 0, 1, 2>>);

    static_assert(std::is_same_v<types_push_front_t<char, std::tuple<>>, tuple1_c>);
    static_assert(std::is_same_v<types_push_front_t<float, tuple1_c>, std::tuple<float, char>>);
    static_assert(std::is_same_v<types_push_front_t<test_type<777>, test_tuple<666>>, test_tuple<777, 666>>);
    static_assert(std::is_same_v<types_push_front_t<test_type<5>, test_tuple<6, 7>>, test_tuple<5, 6, 7>>);

    static_assert(std::is_same_v<types_push_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<types_push_back_t<tuple1_c, float>, std::tuple<char, float>>);
    static_assert(std::is_same_v<types_push_back_t<test_tuple<666>, test_type<777>>, test_tuple<666, 777>>);
    static_assert(std::is_same_v<types_push_back_t<test_tuple<6, 7>, test_type<8>>, test_tuple<6, 7, 8>>);

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
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<>, test_tuple<0, 1, 2>, test_tuple<3, 4, 5, 6>>, test_tuple_0_7>);
    static_assert(std::is_same_v<tuple_cat_t<test_tuple<0, 1>, test_tuple<2, 3, 4>, test_tuple<5, 6, 7, 8>>, test_tuple_0_9>);

    static_assert(0u == min_types_index_element_v<less_sizeof, tuple1_c>);
    static_assert(0u == min_types_index_element_v<less_eq_sizeof, tuple1_c>);
    static_assert(0u == min_types_index_element_v<greater_sizeof, tuple1_c>);
    static_assert(0u == min_types_index_element_v<greater_eq_sizeof, tuple1_c>);
    static_assert(0u == min_types_index_element_v<less_sizeof, tuple2_cd>);
    static_assert(0u == min_types_index_element_v<less_eq_sizeof, tuple2_cd>);
    static_assert(1u == min_types_index_element_v<greater_sizeof, tuple2_cd>);
    static_assert(1u == min_types_index_element_v<greater_eq_sizeof, tuple2_cd>);
    static_assert(4u == min_types_index_element_v<less_sizeof, tuple6_ci16cdcf>);
    static_assert(0u == min_types_index_element_v<less_eq_sizeof, tuple6_ci16cdcf>);
    static_assert(3u == min_types_index_element_v<greater_sizeof, tuple6_ci16cdcf>);
    static_assert(3u == min_types_index_element_v<greater_eq_sizeof, tuple6_ci16cdcf>);
    static_assert(5u == min_types_index_element_v<less_sizeof, tuple7_ci16cddcf>);
    static_assert(0u == min_types_index_element_v<less_eq_sizeof, tuple7_ci16cddcf>);
    static_assert(4u == min_types_index_element_v<greater_sizeof, tuple7_ci16cddcf>);
    static_assert(3u == min_types_index_element_v<greater_eq_sizeof, tuple7_ci16cddcf>);

    using small2big_arithmetic_tuple6 = std::tuple<char, char, char, int16_t, float, double>;
    using big2small_arithmetic_tuple6 = std::tuple<double, float, int16_t, char, char, char>;

    using small2big_arithmetic_tuple7 = std::tuple<char, char, char, int16_t, float, double, double>;
    using big2small_arithmetic_tuple7 = std::tuple<double, double, float, int16_t, char, char, char>;

    static_assert(std::is_same_v<sort_types_t<greater_eq_sizeof, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<sort_types_t<less_sizeof, tuple6_ci16cdcf>, small2big_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_types_t<less_eq_sizeof, tuple6_ci16cdcf>, small2big_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_types_t<greater_sizeof, tuple6_ci16cdcf>, big2small_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_types_t<greater_eq_sizeof, tuple6_ci16cdcf>, big2small_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_types_t<less_sizeof, tuple7_ci16cddcf>, small2big_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_types_t<less_eq_sizeof, tuple7_ci16cddcf>, small2big_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_types_t<greater_sizeof, tuple7_ci16cddcf>, big2small_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_types_t<greater_eq_sizeof, tuple7_ci16cddcf>, big2small_arithmetic_tuple7>);

    static_assert(!std::is_same_v<char32_t, int32_t>);
    static_assert(!std::is_same_v<char32_t, uint32_t>);
    static_assert(4u == std::max(std::max(std::max(sizeof(char32_t), sizeof(int32_t)), sizeof(uint32_t)), sizeof(float)));
    using arithmetic_eq_sizeof_tuple = std::tuple<char32_t, int32_t, uint32_t, float>;

    static_assert(std::is_same_v<sort_types_t<less_eq_sizeof, arithmetic_eq_sizeof_tuple>, arithmetic_eq_sizeof_tuple>);
    static_assert(std::is_same_v<sort_types_t<greater_eq_sizeof, arithmetic_eq_sizeof_tuple>, arithmetic_eq_sizeof_tuple>);

#ifdef _MSC_VER
    static_assert(sizeof(tuple_sizeof_optimization_t<tuple7_ci16cddcf>) < sizeof(tuple7_ci16cddcf));
#else
    static_assert(sizeof(tuple_sizeof_optimization_t<tuple7_ci16cddcf>) <= sizeof(tuple7_ci16cddcf));
#endif    

    static_assert(std::is_same_v<tuple_unique_push_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_push_back_t<tuple1_c, char>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_push_back_t<tuple1_c, double>, std::tuple<char, double>>);

    static_assert(std::is_same_v<tuple_unique_insert_back_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<tuple_unique_insert_back_t<tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_insert_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_insert_back_t<tuple1_c, char>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_insert_back_t<std::tuple<>, char, char>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_insert_back_t<tuple1_c, char, char>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_insert_back_t<std::tuple<>, char, char, char, double, char, char>, std::tuple<char, double>>);
    static_assert(std::is_same_v<tuple_unique_insert_back_t<tuple1_c, char, char, char, double, char, char>, std::tuple<char, double>>);

    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<std::tuple<>, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<tuple1_c, std::tuple<>>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<tuple1_c, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<tuple_unique_push_back_tuple_t<tuple1_c, std::tuple<char, int>>, std::tuple<char, int>>);
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

    static_assert(std::is_same_v<tuple_unique_insert_back_tuple_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<tuple_unique_insert_back_tuple_t<std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<tuple_unique_insert_back_tuple_t<std::tuple<>, std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<tuple_unique_insert_back_tuple_t<std::tuple<>, test_tuple<1, 2, 5>, test_tuple<1, 2, 3, 5, 6>>, test_tuple<1, 2, 5, 3, 6>>);
    static_assert(std::is_same_v<tuple_unique_insert_back_tuple_t<test_tuple<0, 1>, test_tuple<0, 2, 3>, test_tuple<1, 4, 5>, test_tuple<6, 2, 7>>, test_tuple_0_8>);

    D_ASSERT(!errno);
}
