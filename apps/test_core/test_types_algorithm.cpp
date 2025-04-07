#include <core/types_algorithm.h>

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

    struct test_subtypes_tuple_base
    {
        char ch;
        float f;
        types_pack<> tp0;
        std::tuple<> tu0;
        types_pack<types_pack<>> tptp0;

        using subtypes_pack = types_pack<char, float, types_pack<>, std::tuple<>, types_pack<types_pack<>> >;

        template<class Fn>
        constexpr decltype(auto) apply(Fn fn) noexcept
        {
            return fn(ch, f, tp0, tu0, tptp0);
        }
    };

    struct test_subtypes_tuple : test_subtypes_tuple_base
    {
        template<class Fn>
        constexpr decltype(auto) apply(Fn fn) noexcept
        {
            return fn(ch, f, tp0, tu0, tptp0);
        }
    };

    struct test_csubtypes_tuple : test_subtypes_tuple_base
    {
        template<class Fn>
        constexpr decltype(auto) apply(Fn fn) const noexcept
        {
            return fn(ch, f, tp0, tu0, tptp0);
        }
    };

    struct test_static_subtypes_tuple
    {
        static constexpr char ch{};
        static constexpr float f{};
        static constexpr types_pack<> tp0{};
        static constexpr std::tuple<> tu0{};
        static constexpr types_pack<types_pack<>> tptp0{};

        using subtypes_pack = types_pack<char, float, types_pack<>, std::tuple<>, types_pack<types_pack<>> >;

        template<class Fn>
        static constexpr decltype(auto) apply(Fn fn) noexcept
        {
            return fn(ch, f, tp0, tu0, tptp0);
        }
    };
}


void test_tuple_algorithm() noexcept
{
    using types_pack_c_l = types_pack<char, long>;
    using types_pack_s_c_l = types_pack<short, char, long>;
    using tuple_types_pack_tu_v2_pa = tuples_pack<std::tuple, vec2, types_pack>;

    using test_tuple_0_4 = test_tuple<0, 1, 2, 3>;
    using test_tuple_0_5 = test_tuple<0, 1, 2, 3, 4>;
    using test_tuple_0_6 = test_tuple<0, 1, 2, 3, 4, 5>;
    using test_tuple_0_7 = test_tuple<0, 1, 2, 3, 4, 5, 6>;
    using test_tuple_0_8 = test_tuple<0, 1, 2, 3, 4, 5, 6, 7>;
    using test_tuple_0_9 = test_tuple<0, 1, 2, 3, 4, 5, 6, 7, 8>;
    using seq0 = std::index_sequence<>;
    using seq1 = std::make_index_sequence<1u>;
    using seq2 = std::make_index_sequence<2u>;
    using seq12 = std::index_sequence<1u>;
    using seq3 = std::make_index_sequence<3u>;
    using seq13 = std::index_sequence<1u, 2u>;
    using seq31 = std::index_sequence<2u, 1u>;
    using seq4 = std::make_index_sequence<4u>;
    using seq14 = std::index_sequence<1u, 2u, 3u>;

    using tuple0 = std::tuple<>;
    using tuple1_c = std::tuple<char>;
    using tuple1_d = std::tuple<double>;
    using tuple1_f = std::tuple<float>;
    using tuple2_cc = std::tuple<char, char>;
    using tuple2_cl = std::tuple<char, long>;
    using tuple2_cd = std::tuple<char, double>;
    using tuple2_df = std::tuple<double, float>;
    using tuple3_cdf = std::tuple<char, double, float>;
    using tuple2_scl = std::tuple<short, char, long>;
    using tuple6_ci16cdcf = std::tuple<char, int16_t, char, double, char, float>;
    using tuple7_ci16cddcf = std::tuple<char, int16_t, char, double, double, char, float>;

    using unique_resource10 = unique_resource<test_self_call<1>, test_self_call<0>>;
    using unique_resource01 = unique_resource<test_self_call<0>, test_self_call<1>>;

    static_assert(is_same_types_template_v<unique_resource, unique_resource>);
    static_assert(is_same_types_template_v<shared_resource, shared_resource>);
    static_assert(!is_same_types_template_v<unique_resource, shared_resource>);
    static_assert(!is_same_types_template_v<shared_resource, unique_resource>);
    static_assert(is_same_types_template_v<std::tuple, std::tuple>);
    static_assert(is_same_types_template_v<types_pack, types_pack>);
    static_assert(!is_same_types_template_v<std::tuple, types_pack>);

    static_assert(types_template_is_v<types_pack_s_c_l, types_pack>);
    static_assert(!types_template_is_v<types_pack_s_c_l, std::tuple>);
    static_assert(types_template_is_v<tuple7_ci16cddcf, std::tuple>);
    static_assert(!types_template_is_v<tuple7_ci16cddcf, types_pack>);
    static_assert(types_template_is_v<unique_resource10, unique_resource>);
    static_assert(!types_template_is_v<unique_resource10, shared_resource>);
    static_assert(types_template_is_v<shared_resource<int, nothing>, shared_resource>);
    static_assert(!types_template_is_v<shared_resource<int, nothing>, unique_resource>);

    static_assert(std::is_same_v<add_template_t<types_pack, types_pack<>>, types_pack<> >);
    static_assert(std::is_same_v<add_template_t<types_pack, types_pack<char>>, types_pack<char> >);
    static_assert(std::is_same_v<add_template_t<types_pack, types_pack<types_pack<>>>, types_pack<types_pack<>> >);
    static_assert(std::is_same_v<add_template_t<types_pack, types_pack<tuple0>>, types_pack<tuple0> >);
    static_assert(std::is_same_v<add_template_t<types_pack, tuple0>, types_pack<tuple0> >);
    static_assert(std::is_same_v<add_template_t<std::tuple, tuple0>, tuple0>);
    static_assert(std::is_same_v<add_template_t<std::tuple, tuple1_c>, tuple1_c>);

    static_assert(std::is_same_v<tuple_types_pack_tu_v2_pa, tuples_pack<std::tuple, vec2, types_pack>>);
    static_assert(std::is_same_v<tuples_element_tuple_pack_t<0u, std::tuple, vec2>, tuple_pack<std::tuple> >);
    static_assert(std::is_same_v<tuples_element_tuple_pack_t<1u, std::tuple, vec2>, tuple_pack<vec2> >);
    static_assert(std::is_same_v<make_tuples_pack_element_t<0u, tuple_types_pack_tu_v2_pa>, std::tuple<>>);
    static_assert(std::is_same_v<make_tuples_pack_element_t<0u, tuple_types_pack_tu_v2_pa, char>, tuple1_c>);
    static_assert(std::is_same_v<make_tuples_pack_element_t<0u, tuple_types_pack_tu_v2_pa, char, double, float>, tuple3_cdf>);
    static_assert(std::is_same_v<make_tuples_pack_element_t<1u, tuple_types_pack_tu_v2_pa, char>, vec2<char>>);
    static_assert(std::is_same_v<make_tuples_pack_element_t<1u, tuple_types_pack_tu_v2_pa, float>, vec2<float>>);
    static_assert(std::is_same_v<make_tuples_pack_element_t<1u, tuple_types_pack_tu_v2_pa, double>, vec2<double>>);
    static_assert(std::is_same_v<make_tuples_pack_element_t<2u, tuple_types_pack_tu_v2_pa, short, char, long>, types_pack_s_c_l>);
    static_assert(std::is_same_v<make_tuples_pack_element_t<2u, tuple_types_pack_tu_v2_pa>, types_pack<> >);

    static_assert(0u == types_size_v<char>);
    static_assert(0u == types_size_v<std::tuple<>>);
    static_assert(1u == types_size_v<tuple1_c>);
    static_assert(2u == types_size_v<tuple2_cc>);
    static_assert(6u == types_size_v<tuple6_ci16cdcf>);
    static_assert(6u == types_size_v<tuple6_ci16cdcf>);

    static_assert(0u == types_sequence_size_v<seq0>);
    static_assert(1u == types_sequence_size_v<seq1>);
    static_assert(1u == types_sequence_size_v<seq12>);
    static_assert(2u == types_sequence_size_v<seq2>);
    static_assert(2u == types_sequence_size_v<seq13>);
    static_assert(3u == types_sequence_size_v<seq3>);

    static_assert(2u == types_count_if_v<std::is_integral, tuple2_cc>);
    static_assert(1u == types_count_if_v<std::is_integral, tuple2_cd>);
    static_assert(4u == types_count_if_v<std::is_integral, tuple7_ci16cddcf>);
    static_assert(3u == types_count_if_v<std::is_floating_point, tuple7_ci16cddcf>);

    static_assert(std::is_same_v<subtypes_t<test_subtypes_tuple>, test_subtypes_tuple::subtypes_pack>);
    static_assert(std::is_same_v<subtypes_t<test_csubtypes_tuple>, test_csubtypes_tuple::subtypes_pack>);
    static_assert(std::is_same_v<subtypes_t<test_static_subtypes_tuple>, test_static_subtypes_tuple::subtypes_pack>);

    static_assert(std::is_same_v<repack_types_t<std::tuple<>, types_pack>, types_pack<> >);
    static_assert(std::is_same_v<repack_types_t<types_pack<>, std::tuple>, std::tuple<> >);
    static_assert(std::is_same_v<repack_types_t<std::tuple<char>, types_pack>, types_pack<char> >);
    static_assert(std::is_same_v<repack_types_t<std::tuple<long>, types_pack>, types_pack<long> >);
    static_assert(std::is_same_v<repack_types_t<tuple2_cl, types_pack>, types_pack_c_l>);
    static_assert(std::is_same_v<repack_types_t<tuple2_scl, types_pack>, types_pack_s_c_l>);

    static_assert(std::is_same_v<rewrite_types_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<rewrite_types_t<std::tuple<>, char>, std::tuple<char>>);
    static_assert(std::is_same_v<rewrite_types_t<std::tuple<>, char, double, float>, tuple3_cdf>);
    static_assert(std::is_same_v<rewrite_types_t<tuple7_ci16cddcf, char, double, float>, tuple3_cdf>);
    static_assert(std::is_same_v<rewrite_types_t<tuple7_ci16cddcf>, std::tuple<>>);

    static_assert(std::is_same_v<clear_types_t<std::tuple<> >, std::tuple<>>);
    static_assert(std::is_same_v<clear_types_t<tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<clear_types_t<tuple2_cc>, std::tuple<>>);
    static_assert(std::is_same_v<clear_types_t<tuple2_cd>, std::tuple<>>);
    static_assert(std::is_same_v<clear_types_t<tuple7_ci16cddcf>, std::tuple<>>);

    static_assert(std::is_same_v<dummy, transform_types_t<std::add_const_t, dummy>>);
    static_assert(std::is_same_v<std::tuple<>, transform_types_t<std::add_const_t, std::tuple<>>>);
    static_assert(std::is_same_v<std::tuple<const char>, transform_types_t<std::add_const_t, tuple1_c>>);
    static_assert(std::is_same_v<std::tuple<const double, const char>, transform_types_t<std::add_const_t, std::tuple<double, char>>>);

    static_assert(!types_has_v<std::is_integral, std::tuple<> >);
    static_assert(!types_has_v<std::is_integral, tuple1_d>);
    static_assert(types_has_v<std::is_integral, tuple1_c>);
    static_assert(types_has_v<std::is_integral, tuple2_cc>);
    static_assert(types_has_v<std::is_integral, tuple2_cd>);
    static_assert(!types_has_v<std::is_integral, tuple2_df>);

    static_assert(!types_has_v<std::is_floating_point, std::tuple<> >);
    static_assert(types_has_v<std::is_floating_point, tuple1_d>);
    static_assert(!types_has_v<std::is_floating_point, tuple1_c>);
    static_assert(!types_has_v<std::is_floating_point, tuple2_cc>);
    static_assert(types_has_v<std::is_floating_point, tuple2_cd>);
    static_assert(types_has_v<std::is_floating_point, tuple2_df>);

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
    static_assert(std::is_same_v<short, types_pack_element_t<0u, types_pack_s_c_l>>);
    static_assert(std::is_same_v<char, types_pack_element_t<1u, types_pack_s_c_l>>);
    static_assert(std::is_same_v<long, types_pack_element_t<2u, types_pack_s_c_l>>);

    static_assert(std::is_same_v<types_sol_t<std::tuple<> >, std::tuple<>>);
    static_assert(std::is_same_v<types_sol_t<std::tuple<tuple1_c, std::tuple<>> >, tuple1_c>);
    static_assert(std::is_same_v<types_sol_t<std::tuple<std::tuple<>, tuple1_c> >, tuple1_c>);
    static_assert(std::is_same_v<types_sol_t<std::tuple<tuple1_c, tuple1_d> >, tuple2_cd>);
    static_assert(std::is_same_v<types_sol_t<std::tuple<tuple1_c, tuple1_d, tuple1_f> >, tuple3_cdf>);
    static_assert(std::is_same_v<types_sol_t<std::tuple<tuple2_cd, tuple1_f> >, tuple3_cdf>);

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

    static_assert(std::is_same_v<index_sequence_pop_front_t<seq1>, seq0>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<seq2>, seq12>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<seq3>, seq13>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<seq4>, seq14>);

    static_assert(std::is_same_v<types_pop_front_t<tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<types_pop_front_t<tuple2_cd>, tuple1_d>);
    static_assert(std::is_same_v<types_pop_front_t<test_tuple_0_5>, test_tuple<1, 2, 3, 4>>);
    static_assert(std::is_same_v<types_pop_front_t<test_tuple<333, 666, 777>>, test_tuple<666, 777>>);

    static_assert(std::is_same_v<types_pop_front_t<types_pack_s_c_l>, types_pack_c_l>);
    static_assert(std::is_same_v<types_pop_front_t<types_pack_c_l>, types_pack<long>>);
    static_assert(std::is_same_v<types_pop_front_t<types_pack<long>>, types_pack<>>);

    static_assert(std::is_same_v<index_sequence_push_front_t<33, seq3>, std::index_sequence<33, 0, 1, 2>>);

    static_assert(std::is_same_v<types_pop_back_t<tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<types_pop_back_t<tuple2_cd>, std::tuple<char>>);
    static_assert(std::is_same_v<types_pop_back_t<test_tuple_0_5>, test_tuple_0_4>);
    static_assert(std::is_same_v<types_pop_back_t<test_tuple<333, 666, 777>>, test_tuple<333, 666>>);

    static_assert(std::is_same_v<types_push_front_t<char, std::tuple<>>, tuple1_c>);
    static_assert(std::is_same_v<types_push_front_t<float, tuple1_c>, std::tuple<float, char>>);
    static_assert(std::is_same_v<types_push_front_t<test_type<777>, test_tuple<666>>, test_tuple<777, 666>>);
    static_assert(std::is_same_v<types_push_front_t<test_type<5>, test_tuple<6, 7>>, test_tuple<5, 6, 7>>);

    static_assert(std::is_same_v<types_insert_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<types_insert_back_t<tuple1_c, float>, std::tuple<char, float>>);
    static_assert(std::is_same_v<types_insert_back_t<test_tuple<666>, test_type<777>>, test_tuple<666, 777>>);
    static_assert(std::is_same_v<types_insert_back_t<test_tuple<6, 7>, test_type<8>>, test_tuple<6, 7, 8>>);

    static_assert(std::is_same_v<types_insert_back_t<std::tuple<>, char, char>, tuple2_cc>);
    static_assert(std::is_same_v<types_insert_back_t<std::tuple<>, char, double>, tuple2_cd>);
    static_assert(std::is_same_v<types_insert_back_t<tuple1_c, double, float>, tuple3_cdf>);
    static_assert(std::is_same_v<types_insert_back_t<test_tuple<666>, test_type<777>, test_type<8>>, test_tuple<666, 777, 8>>);
    static_assert(std::is_same_v<types_insert_back_t<test_tuple<6, 7>, test_type<8>, test_type<777>>, test_tuple<6, 7, 8, 777>>);

    static_assert(std::is_same_v<types_select_t<std::is_integral, std::tuple<> >, tuple0>);
    static_assert(std::is_same_v<types_select_t<std::is_integral, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<types_select_t<std::is_integral, tuple1_d>, tuple0>);
    static_assert(std::is_same_v<types_select_t<std::is_floating_point, tuple1_c>, tuple0>);
    static_assert(std::is_same_v<types_select_t<std::is_floating_point, tuple1_d>, tuple1_d>);
    static_assert(std::is_same_v<types_select_t<std::is_floating_point, tuple6_ci16cdcf>, tuple2_df>);

    static_assert(std::is_same_v<types_cat_for_t<std::tuple, std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, std::tuple<>, char>, std::tuple<char>>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, char, std::tuple<>>, std::tuple<char>>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, std::tuple<>, std::tuple<>, char>, std::tuple<char>>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, std::tuple<>, char, std::tuple<>>, std::tuple<char>>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, char, std::tuple<>, std::tuple<>>, std::tuple<char>>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, char, char>, tuple2_cc>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, char, double>, tuple2_cd>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, std::tuple<>, test_tuple_0_5>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0>, test_tuple<1, 2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_type<0>, test_tuple<1, 2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_tuple<2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1, 2>, test_tuple<3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1, 2, 3>, test_tuple<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1, 2, 3, 4>, std::tuple<>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, std::tuple<>, std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0>, test_tuple<1>, test_tuple<2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_type<0>, test_type<1>, test_tuple<2, 3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0>, test_tuple<1, 2, 3>, test_tuple<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_type<0>, test_tuple<1, 2, 3>, test_type<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1, 2>, test_tuple<3>, test_tuple<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1, 2>, test_type<3>, test_type<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_tuple<2, 3>, test_tuple<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_tuple<2, 3>, test_type<4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_tuple<2>, test_tuple<3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_type<2>, test_tuple<3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0>, test_tuple<1, 2>, test_tuple<3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_type<0>, test_tuple<1, 2>, test_tuple<3, 4>>, test_tuple_0_5>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_tuple<2, 3>, test_tuple<4, 5>>, test_tuple_0_6>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_tuple<2, 3, 4, 5>, std::tuple<>>, test_tuple_0_6>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_tuple<>, test_tuple<2, 3, 4, 5>>, test_tuple_0_6>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<>, test_tuple<0, 1>, test_tuple<2, 3, 4, 5>>, test_tuple_0_6>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<>, test_tuple<0, 1, 2>, test_tuple<3, 4, 5, 6>>, test_tuple_0_7>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0>, test_tuple<1, 2, 3>, test_tuple<4, 5, 6, 7>>, test_tuple_0_8>);
    static_assert(std::is_same_v<types_cat_for_t<std::tuple, test_tuple<0, 1>, test_tuple<2, 3, 4>, test_tuple<5, 6, 7, 8>>, test_tuple_0_9>);

    static_assert(std::is_same_v<types_sol_t<vec2<int>>, vec2<int>>);
    static_assert(std::is_same_v<types_sol_t<vec2<vec2<int>>>, vec2<int>>);

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
    static_assert(sizeof(types_sizeof_optimization_t<tuple7_ci16cddcf>) < sizeof(tuple7_ci16cddcf));
#else
    static_assert(sizeof(types_sizeof_optimization_t<tuple7_ci16cddcf>) <= sizeof(tuple7_ci16cddcf));
#endif    

    static_assert(std::is_same_v<types_unique_push_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_push_back_t<tuple1_c, char>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_push_back_t<tuple1_c, double>, std::tuple<char, double>>);

    static_assert(std::is_same_v<types_unique_insert_back_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<types_unique_insert_back_t<tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_insert_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_insert_back_t<tuple1_c, char>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_insert_back_t<std::tuple<>, char, char>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_insert_back_t<tuple1_c, char, char>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_insert_back_t<std::tuple<>, char, char, char, double, char, char>, std::tuple<char, double>>);
    static_assert(std::is_same_v<types_unique_insert_back_t<tuple1_c, char, char, char, double, char, char>, std::tuple<char, double>>);

    static_assert(std::is_same_v<types_unique_push_back_pack_t<std::tuple, std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<types_unique_push_back_pack_t<std::tuple, std::tuple<>, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_push_back_pack_t<std::tuple, tuple1_c, std::tuple<>>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_push_back_pack_t<std::tuple, tuple1_c, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<types_unique_push_back_pack_t<std::tuple, tuple1_c, std::tuple<char, int>>, std::tuple<char, int>>);
    static_assert(std::is_same_v<types_unique_push_back_pack_t<std::tuple, test_tuple<1, 2, 5>, test_tuple_0_6>, test_tuple<1, 2, 5, 0, 3, 4>>);


    {
        using test_unique_t = test_tuple<0, 1, 2>;

        static_assert(std::is_same_v<types_unique_t<std::tuple<>>, std::tuple<>>);
        static_assert(std::is_same_v<types_unique_t<test_unique_t>, test_unique_t>);
        static_assert(std::is_same_v<types_unique_t<test_tuple<0, 1, 1, 2>>, test_unique_t>);
        static_assert(std::is_same_v<types_unique_t<test_tuple<0, 1, 1, 2, 2>>, test_unique_t>);
        static_assert(std::is_same_v<types_unique_t<test_tuple<0, 1, 0, 2>>, test_unique_t>);
        static_assert(std::is_same_v<types_unique_t<test_tuple<0, 1, 0, 2, 1>>, test_unique_t>);
        static_assert(std::is_same_v<types_unique_t<test_tuple<0, 1, 0, 2, 1, 2>>, test_unique_t>);
        static_assert(std::is_same_v<types_unique_t<test_tuple<0, 0, 1, 2>>, test_unique_t>);
    }

    D_ASSERT(!errno);
}
