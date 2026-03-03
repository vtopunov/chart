#include <core/types_algorithm.h>

#include <core/view.h>

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
        ttypes<> tp0;
        std::tuple<> tu0;
        ttypes<ttypes<>> tptp0;

        using subttypes = ttypes<char, float, ttypes<>, std::tuple<>, ttypes<ttypes<>> >;

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
        static constexpr ttypes<> tp0{};
        static constexpr std::tuple<> tu0{};
        static constexpr ttypes<ttypes<>> tptp0{};

        using subttypes = ttypes<char, float, ttypes<>, std::tuple<>, ttypes<ttypes<>> >;

        template<class Fn>
        static constexpr decltype(auto) apply(Fn fn) noexcept
        {
            return fn(ch, f, tp0, tu0, tptp0);
        }
    };
}


void test_tuple_algorithm() noexcept
{
    using ttypes_c_l = ttypes<char, long>;
    using ttypes_s_c_l = ttypes<short, char, long>;
    using tuple_ttypes_tu_v2_pa = ttuples<std::tuple, vec2, ttypes>;

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

    static_assert(std::is_same_v<ttypes<float, float>, ttypes_function_t<decltype(std::sinf)> >);
    static_assert(std::is_same_v<ttypes<void>, ttypes_function_t<void()> >);
    static_assert(std::is_same_v<ttypes<int>, ttypes_function_t<int()> >);
    static_assert(std::is_same_v<ttypes<int>, ttypes_function_t<int() noexcept> >);
    static_assert(std::is_same_v<ttypes<int>, ttypes_function_t<int(*)() noexcept> >);
    static_assert(std::is_same_v<ttypes<int>, ttypes_function_t<int(*const)() noexcept> >);
    static_assert(std::is_same_v<ttypes<int, ttypes<> >, ttypes_function_t<int(* const)(ttypes<>) noexcept> >);
    static_assert(std::is_same_v<ttypes<char, int, long, short>, ttypes_function_t<char(int, long, short)> >);

    static_assert(is_same_template_v<unique_resource, unique_resource>);
    static_assert(is_same_template_v<shared_resource, shared_resource>);
    static_assert(!is_same_template_v<unique_resource, shared_resource>);
    static_assert(!is_same_template_v<shared_resource, unique_resource>);
    static_assert(is_same_template_v<std::tuple, std::tuple>);
    static_assert(is_same_template_v<ttypes, ttypes>);
    static_assert(!is_same_template_v<std::tuple, ttypes>);

    static_assert(template_is_v<ttypes_s_c_l, ttypes>);
    static_assert(!template_is_v<ttypes_s_c_l, std::tuple>);
    static_assert(template_is_v<tuple7_ci16cddcf, std::tuple>);
    static_assert(!template_is_v<tuple7_ci16cddcf, ttypes>);
    static_assert(template_is_v<unique_resource10, unique_resource>);
    static_assert(!template_is_v<unique_resource10, shared_resource>);
    static_assert(template_is_v<shared_resource<int, nothing>, shared_resource>);
    static_assert(!template_is_v<shared_resource<int, nothing>, unique_resource>);

    static_assert(std::is_same_v<add_template_t<ttypes, ttypes<>>, ttypes<> >);
    static_assert(std::is_same_v<add_template_t<ttypes, ttypes<char>>, ttypes<char> >);
    static_assert(std::is_same_v<add_template_t<ttypes, ttypes<ttypes<>>>, ttypes<ttypes<>> >);
    static_assert(std::is_same_v<add_template_t<ttypes, ttypes<tuple0>>, ttypes<tuple0> >);
    static_assert(std::is_same_v<add_template_t<ttypes, tuple0>, ttypes<tuple0> >);
    static_assert(std::is_same_v<add_template_t<std::tuple, tuple0>, tuple0>);
    static_assert(std::is_same_v<add_template_t<std::tuple, tuple1_c>, tuple1_c>);

    static_assert(std::is_same_v<tuple_ttypes_tu_v2_pa, ttuples<std::tuple, vec2, ttypes>>);
    static_assert(std::is_same_v<tuples_element_t<0u, std::tuple, vec2>, ttuples<std::tuple> >);
    static_assert(std::is_same_v<tuples_element_t<1u, std::tuple, vec2>, ttuples<vec2> >);
    static_assert(std::is_same_v<make_ttuples_element_t<0u, tuple_ttypes_tu_v2_pa>, std::tuple<>>);
    static_assert(std::is_same_v<make_ttuples_element_t<0u, tuple_ttypes_tu_v2_pa, char>, tuple1_c>);
    static_assert(std::is_same_v<make_ttuples_element_t<0u, tuple_ttypes_tu_v2_pa, char, double, float>, tuple3_cdf>);
    static_assert(std::is_same_v<make_ttuples_element_t<1u, tuple_ttypes_tu_v2_pa, char>, vec2<char>>);
    static_assert(std::is_same_v<make_ttuples_element_t<1u, tuple_ttypes_tu_v2_pa, float>, vec2<float>>);
    static_assert(std::is_same_v<make_ttuples_element_t<1u, tuple_ttypes_tu_v2_pa, double>, vec2<double>>);
    static_assert(std::is_same_v<make_ttuples_element_t<2u, tuple_ttypes_tu_v2_pa, short, char, long>, ttypes_s_c_l>);
    static_assert(std::is_same_v<make_ttuples_element_t<2u, tuple_ttypes_tu_v2_pa>, ttypes<> >);

    static_assert(0u == ttypes_size_v<char>);
    static_assert(0u == ttypes_size_v<std::tuple<>>);
    static_assert(1u == ttypes_size_v<tuple1_c>);
    static_assert(2u == ttypes_size_v<tuple2_cc>);
    static_assert(6u == ttypes_size_v<tuple6_ci16cdcf>);
    static_assert(6u == ttypes_size_v<tuple6_ci16cdcf>);

    static_assert(0u == ttypes_sequence_size_v<seq0>);
    static_assert(1u == ttypes_sequence_size_v<seq1>);
    static_assert(1u == ttypes_sequence_size_v<seq12>);
    static_assert(2u == ttypes_sequence_size_v<seq2>);
    static_assert(2u == ttypes_sequence_size_v<seq13>);
    static_assert(3u == ttypes_sequence_size_v<seq3>);

    static_assert(2u == ttypes_count_if_v<std::is_integral, tuple2_cc>);
    static_assert(1u == ttypes_count_if_v<std::is_integral, tuple2_cd>);
    static_assert(4u == ttypes_count_if_v<std::is_integral, tuple7_ci16cddcf>);
    static_assert(3u == ttypes_count_if_v<std::is_floating_point, tuple7_ci16cddcf>);

    static_assert(std::is_same_v<subtypes_t<test_subtypes_tuple>, test_subtypes_tuple::subttypes>);
    static_assert(std::is_same_v<subtypes_t<test_csubtypes_tuple>, test_csubtypes_tuple::subttypes>);
    static_assert(std::is_same_v<subtypes_t<test_static_subtypes_tuple>, test_static_subtypes_tuple::subttypes>);

    static_assert(std::is_same_v<ttypes_repack_t<std::tuple<>, ttypes>, ttypes<> >);
    static_assert(std::is_same_v<ttypes_repack_t<ttypes<>, std::tuple>, std::tuple<> >);
    static_assert(std::is_same_v<ttypes_repack_t<std::tuple<char>, ttypes>, ttypes<char> >);
    static_assert(std::is_same_v<ttypes_repack_t<std::tuple<long>, ttypes>, ttypes<long> >);
    static_assert(std::is_same_v<ttypes_repack_t<tuple2_cl, ttypes>, ttypes_c_l>);
    static_assert(std::is_same_v<ttypes_repack_t<tuple2_scl, ttypes>, ttypes_s_c_l>);

    static_assert(std::is_same_v<ttypes_rewrite_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_rewrite_t<std::tuple<>, char>, std::tuple<char>>);
    static_assert(std::is_same_v<ttypes_rewrite_t<std::tuple<>, char, double, float>, tuple3_cdf>);
    static_assert(std::is_same_v<ttypes_rewrite_t<tuple7_ci16cddcf, char, double, float>, tuple3_cdf>);
    static_assert(std::is_same_v<ttypes_rewrite_t<tuple7_ci16cddcf>, std::tuple<>>);

    static_assert(std::is_same_v<ttypes_clear_t<std::tuple<> >, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_clear_t<tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_clear_t<tuple2_cc>, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_clear_t<tuple2_cd>, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_clear_t<tuple7_ci16cddcf>, std::tuple<>>);

    static_assert(std::is_same_v<dummy, ttypes_transform_t<std::add_const_t, dummy>>);
    static_assert(std::is_same_v<std::tuple<>, ttypes_transform_t<std::add_const_t, std::tuple<>>>);
    static_assert(std::is_same_v<std::tuple<const char>, ttypes_transform_t<std::add_const_t, tuple1_c>>);
    static_assert(std::is_same_v<std::tuple<const double, const char>, ttypes_transform_t<std::add_const_t, std::tuple<double, char>>>);

    static_assert(!ttypes_has_v<std::is_integral, std::tuple<> >);
    static_assert(!ttypes_has_v<std::is_integral, tuple1_d>);
    static_assert(ttypes_has_v<std::is_integral, tuple1_c>);
    static_assert(ttypes_has_v<std::is_integral, tuple2_cc>);
    static_assert(ttypes_has_v<std::is_integral, tuple2_cd>);
    static_assert(!ttypes_has_v<std::is_integral, tuple2_df>);

    static_assert(!ttypes_has_v<std::is_floating_point, std::tuple<> >);
    static_assert(ttypes_has_v<std::is_floating_point, tuple1_d>);
    static_assert(!ttypes_has_v<std::is_floating_point, tuple1_c>);
    static_assert(!ttypes_has_v<std::is_floating_point, tuple2_cc>);
    static_assert(ttypes_has_v<std::is_floating_point, tuple2_cd>);
    static_assert(ttypes_has_v<std::is_floating_point, tuple2_df>);

    static_assert(ttypes_has_type_v<test_type<0>, test_tuple_0_5>);
    static_assert(ttypes_has_type_v<test_type<4>, test_tuple_0_5>);
    static_assert(!ttypes_has_type_v<test_type<5>, test_tuple_0_5>);
    static_assert(ttypes_has_type_v<char, tuple6_ci16cdcf>);
    static_assert(!ttypes_has_type_v<int32_t, tuple6_ci16cdcf>);
    static_assert(!ttypes_has_type_v<char, std::tuple<>>);
    static_assert(!ttypes_has_type_v<std::tuple<>, std::tuple<>>);
    static_assert(ttypes_has_type_v<std::tuple<>, std::tuple<std::tuple<>>>);

    static_assert(ttypes_has_invoke_v<std::tuple<test_call_type<0>>, test_type<0>>);
    static_assert(!ttypes_has_invoke_v<std::tuple<test_call_type<0>>, test_type<1>>);
    static_assert(ttypes_has_invoke_v<std::tuple<test_call_type<0>, test_call_type<1>>, test_type<1>>);
    static_assert(ttypes_has_invoke_v<std::tuple<test_call_type<0>, test_call_type<0, 1>>, test_type<0>, test_type<1>>);

    static_assert(std::is_same_v<char, types_element_t<0u, char>>);
    static_assert(std::is_same_v<char, types_element_t<0u, char, int>>);
    static_assert(std::is_same_v<int, types_element_t<1u, char, int>>);
    static_assert(std::is_same_v<double, types_element_t<2u, char, int, double>>);
    static_assert(std::is_same_v<int, types_element_t<1u, char, int, double>>);
    static_assert(std::is_same_v<char, types_element_t<0u, char, int, double>>);
    static_assert(std::is_same_v<short, ttypes_element_t<0u, ttypes_s_c_l>>);
    static_assert(std::is_same_v<char, ttypes_element_t<1u, ttypes_s_c_l>>);
    static_assert(std::is_same_v<long, ttypes_element_t<2u, ttypes_s_c_l>>);
    static_assert(0_uz == types_index_element_v<char, char, int, float, double>);
    static_assert(1_uz == types_index_element_v<int, char, int, float, double>);
    static_assert(2_uz == types_index_element_v<float, char, int, float, double>);
    static_assert(3_uz == types_index_element_v<double, char, int, float, double>);
    static_assert(2_uz == types_index_element_v<double, char, int, double, float>);
    static_assert(1_uz == types_index_element_v<char, int, char, double, float>);
    static_assert(0_uz == ttypes_index_element_v<short, ttypes_s_c_l>);
    static_assert(1_uz == ttypes_index_element_v<char, ttypes_s_c_l>);
    static_assert(2_uz == ttypes_index_element_v<long, ttypes_s_c_l>);

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

    static_assert(std::is_same_v<ttypes_reorder_t<test_tuple_0_5, seq4>, test_tuple_0_4>);
    static_assert(std::is_same_v<ttypes_reorder_t<test_tuple_0_5, seq14>, test_tuple<1, 2, 3>>);
    static_assert(std::is_same_v<ttypes_reorder_t<test_tuple_0_5, seq31>, test_tuple<2, 1>>);

    static_assert(std::is_same_v<index_sequence_pop_front_t<seq1>, seq0>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<seq2>, seq12>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<seq3>, seq13>);
    static_assert(std::is_same_v<index_sequence_pop_front_t<seq4>, seq14>);

    static_assert(std::is_same_v<types_front_or_t<dummy, char, int, double>, char>);
    static_assert(std::is_same_v<types_front_or_t<dummy, double, int, char>, double>);
    static_assert(std::is_same_v<types_front_or_t<dummy>, dummy>);

    static_assert(std::is_same_v<ttypes_pop_front_t<tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_pop_front_t<tuple2_cd>, tuple1_d>);
    static_assert(std::is_same_v<ttypes_pop_front_t<test_tuple_0_5>, test_tuple<1, 2, 3, 4>>);
    static_assert(std::is_same_v<ttypes_pop_front_t<test_tuple<333, 666, 777>>, test_tuple<666, 777>>);

    static_assert(std::is_same_v<ttypes_pop_front_t<ttypes_s_c_l>, ttypes_c_l>);
    static_assert(std::is_same_v<ttypes_pop_front_t<ttypes_c_l>, ttypes<long>>);
    static_assert(std::is_same_v<ttypes_pop_front_t<ttypes<long>>, ttypes<>>);

    static_assert(std::is_same_v<index_sequence_push_front_t<33, seq3>, std::index_sequence<33, 0, 1, 2>>);

    static_assert(std::is_same_v<ttypes_pop_back_t<tuple1_c>, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_pop_back_t<tuple2_cd>, std::tuple<char>>);
    static_assert(std::is_same_v<ttypes_pop_back_t<test_tuple_0_5>, test_tuple_0_4>);
    static_assert(std::is_same_v<ttypes_pop_back_t<test_tuple<333, 666, 777>>, test_tuple<333, 666>>);

    static_assert(std::is_same_v<ttypes_push_front_t<char, std::tuple<>>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_push_front_t<float, tuple1_c>, std::tuple<float, char>>);
    static_assert(std::is_same_v<ttypes_push_front_t<test_type<777>, test_tuple<666>>, test_tuple<777, 666>>);
    static_assert(std::is_same_v<ttypes_push_front_t<test_type<5>, test_tuple<6, 7>>, test_tuple<5, 6, 7>>);

    static_assert(std::is_same_v<ttypes_insert_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_insert_back_t<tuple1_c, float>, std::tuple<char, float>>);
    static_assert(std::is_same_v<ttypes_insert_back_t<test_tuple<666>, test_type<777>>, test_tuple<666, 777>>);
    static_assert(std::is_same_v<ttypes_insert_back_t<test_tuple<6, 7>, test_type<8>>, test_tuple<6, 7, 8>>);

    static_assert(std::is_same_v<ttypes_insert_back_t<std::tuple<>, char, char>, tuple2_cc>);
    static_assert(std::is_same_v<ttypes_insert_back_t<std::tuple<>, char, double>, tuple2_cd>);
    static_assert(std::is_same_v<ttypes_insert_back_t<tuple1_c, double, float>, tuple3_cdf>);
    static_assert(std::is_same_v<ttypes_insert_back_t<test_tuple<666>, test_type<777>, test_type<8>>, test_tuple<666, 777, 8>>);
    static_assert(std::is_same_v<ttypes_insert_back_t<test_tuple<6, 7>, test_type<8>, test_type<777>>, test_tuple<6, 7, 8, 777>>);

    static_assert(std::is_same_v<ttypes_select_t<std::is_integral, std::tuple<> >, tuple0>);
    static_assert(std::is_same_v<ttypes_select_t<std::is_integral, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_select_t<std::is_integral, tuple1_d>, tuple0>);
    static_assert(std::is_same_v<ttypes_select_t<std::is_floating_point, tuple1_c>, tuple0>);
    static_assert(std::is_same_v<ttypes_select_t<std::is_floating_point, tuple1_d>, tuple1_d>);
    static_assert(std::is_same_v<ttypes_select_t<std::is_floating_point, tuple6_ci16cdcf>, tuple2_df>);

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

    static_assert(0u == min_ttypes_index_element_v<less_sizeof, tuple1_c>);
    static_assert(0u == min_ttypes_index_element_v<less_eq_sizeof, tuple1_c>);
    static_assert(0u == min_ttypes_index_element_v<greater_sizeof, tuple1_c>);
    static_assert(0u == min_ttypes_index_element_v<greater_eq_sizeof, tuple1_c>);
    static_assert(0u == min_ttypes_index_element_v<less_sizeof, tuple2_cd>);
    static_assert(0u == min_ttypes_index_element_v<less_eq_sizeof, tuple2_cd>);
    static_assert(1u == min_ttypes_index_element_v<greater_sizeof, tuple2_cd>);
    static_assert(1u == min_ttypes_index_element_v<greater_eq_sizeof, tuple2_cd>);
    static_assert(4u == min_ttypes_index_element_v<less_sizeof, tuple6_ci16cdcf>);
    static_assert(0u == min_ttypes_index_element_v<less_eq_sizeof, tuple6_ci16cdcf>);
    static_assert(3u == min_ttypes_index_element_v<greater_sizeof, tuple6_ci16cdcf>);
    static_assert(3u == min_ttypes_index_element_v<greater_eq_sizeof, tuple6_ci16cdcf>);
    static_assert(5u == min_ttypes_index_element_v<less_sizeof, tuple7_ci16cddcf>);
    static_assert(0u == min_ttypes_index_element_v<less_eq_sizeof, tuple7_ci16cddcf>);
    static_assert(4u == min_ttypes_index_element_v<greater_sizeof, tuple7_ci16cddcf>);
    static_assert(3u == min_ttypes_index_element_v<greater_eq_sizeof, tuple7_ci16cddcf>);

    using small2big_arithmetic_tuple6 = std::tuple<char, char, char, int16_t, float, double>;
    using big2small_arithmetic_tuple6 = std::tuple<double, float, int16_t, char, char, char>;

    using small2big_arithmetic_tuple7 = std::tuple<char, char, char, int16_t, float, double, double>;
    using big2small_arithmetic_tuple7 = std::tuple<double, double, float, int16_t, char, char, char>;

    static_assert(std::is_same_v<sort_ttypes_t<greater_eq_sizeof, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<sort_ttypes_t<less_sizeof, tuple6_ci16cdcf>, small2big_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_ttypes_t<less_eq_sizeof, tuple6_ci16cdcf>, small2big_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_ttypes_t<greater_sizeof, tuple6_ci16cdcf>, big2small_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_ttypes_t<greater_eq_sizeof, tuple6_ci16cdcf>, big2small_arithmetic_tuple6>);
    static_assert(std::is_same_v<sort_ttypes_t<less_sizeof, tuple7_ci16cddcf>, small2big_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_ttypes_t<less_eq_sizeof, tuple7_ci16cddcf>, small2big_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_ttypes_t<greater_sizeof, tuple7_ci16cddcf>, big2small_arithmetic_tuple7>);
    static_assert(std::is_same_v<sort_ttypes_t<greater_eq_sizeof, tuple7_ci16cddcf>, big2small_arithmetic_tuple7>);

    static_assert(!std::is_same_v<char32_t, int32_t>);
    static_assert(!std::is_same_v<char32_t, uint32_t>);
    static_assert(4u == std::max(std::max(std::max(sizeof(char32_t), sizeof(int32_t)), sizeof(uint32_t)), sizeof(float)));
    using arithmetic_eq_sizeof_tuple = std::tuple<char32_t, int32_t, uint32_t, float>;

    static_assert(std::is_same_v<sort_ttypes_t<less_eq_sizeof, arithmetic_eq_sizeof_tuple>, arithmetic_eq_sizeof_tuple>);
    static_assert(std::is_same_v<sort_ttypes_t<greater_eq_sizeof, arithmetic_eq_sizeof_tuple>, arithmetic_eq_sizeof_tuple>);

#ifdef _MSC_VER
    static_assert(sizeof(ttypes_sizeof_optimization_t<tuple7_ci16cddcf>) < sizeof(tuple7_ci16cddcf));
#else
    static_assert(sizeof(ttypes_sizeof_optimization_t<tuple7_ci16cddcf>) <= sizeof(tuple7_ci16cddcf));
#endif    

    static_assert(std::is_same_v<ttypes_unique_push_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_push_back_t<tuple1_c, char>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_push_back_t<tuple1_c, double>, std::tuple<char, double>>);

    static_assert(std::is_same_v<ttypes_unique_insert_back_t<std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_unique_insert_back_t<tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_insert_back_t<std::tuple<>, char>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_insert_back_t<tuple1_c, char>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_insert_back_t<std::tuple<>, char, char>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_insert_back_t<tuple1_c, char, char>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_insert_back_t<std::tuple<>, char, char, char, double, char, char>, std::tuple<char, double>>);
    static_assert(std::is_same_v<ttypes_unique_insert_back_t<tuple1_c, char, char, char, double, char, char>, std::tuple<char, double>>);

    static_assert(std::is_same_v<ttypes_unique_push_back_ttypes_t<std::tuple, std::tuple<>, std::tuple<>>, std::tuple<>>);
    static_assert(std::is_same_v<ttypes_unique_push_back_ttypes_t<std::tuple, std::tuple<>, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_push_back_ttypes_t<std::tuple, tuple1_c, std::tuple<>>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_push_back_ttypes_t<std::tuple, tuple1_c, tuple1_c>, tuple1_c>);
    static_assert(std::is_same_v<ttypes_unique_push_back_ttypes_t<std::tuple, tuple1_c, std::tuple<char, int>>, std::tuple<char, int>>);
    static_assert(std::is_same_v<ttypes_unique_push_back_ttypes_t<std::tuple, test_tuple<1, 2, 5>, test_tuple_0_6>, test_tuple<1, 2, 5, 0, 3, 4>>);


    {
        using test_unique_t = test_tuple<0, 1, 2>;

        static_assert(std::is_same_v<ttypes_unique_t<std::tuple<>>, std::tuple<>>);
        static_assert(std::is_same_v<ttypes_unique_t<test_unique_t>, test_unique_t>);
        static_assert(std::is_same_v<ttypes_unique_t<test_tuple<0, 1, 1, 2>>, test_unique_t>);
        static_assert(std::is_same_v<ttypes_unique_t<test_tuple<0, 1, 1, 2, 2>>, test_unique_t>);
        static_assert(std::is_same_v<ttypes_unique_t<test_tuple<0, 1, 0, 2>>, test_unique_t>);
        static_assert(std::is_same_v<ttypes_unique_t<test_tuple<0, 1, 0, 2, 1>>, test_unique_t>);
        static_assert(std::is_same_v<ttypes_unique_t<test_tuple<0, 1, 0, 2, 1, 2>>, test_unique_t>);
        static_assert(std::is_same_v<ttypes_unique_t<test_tuple<0, 0, 1, 2>>, test_unique_t>);
    }

    D_ASSERT(!errno);
}
