#include "core/view.h"
#include "core/assert.h"

#include <cerrno>


namespace
{
    template<size_t N>
    struct bytes_array
    {
        uint8_t content[N];
    };

    template<size_t N>
    struct bytes_array_with_view : bytes_array<N>
    {
        struct view_type
        {};
    };

    template<size_t N>
    struct bytes_array_with_copy_op : bytes_array<N>
    {
        constexpr bytes_array_with_copy_op(const bytes_array_with_copy_op&) noexcept
        {}

        constexpr bytes_array_with_copy_op& operator = (const bytes_array_with_copy_op&) noexcept
        {
            return *this;
        }
    };

    template<size_t N>
    struct bytes_array_with_copy_op_and_view : bytes_array<N>
    {
        struct view_type
        {};

        constexpr bytes_array_with_copy_op_and_view(const bytes_array_with_copy_op_and_view&) noexcept
        {}

        constexpr bytes_array_with_copy_op_and_view& operator = (const bytes_array_with_copy_op_and_view&) noexcept
        {
            return *this;
        }
    };

    template<class T>
    constexpr auto view_by_copy_is_same_v = std::is_same_v<view_by_copy_t<T>, T>;

    template<class T>
    constexpr auto view_by_copy_is_same_cref_v = std::is_same_v<view_by_copy_t<T>, std::add_lvalue_reference_t<std::add_const_t<T>>>;

    template<class T>
    constexpr auto view_is_same_v = std::is_same_v<view_t<T>, T>;

    template<class T>
    constexpr auto view_is_same_cref_v = std::is_same_v<view_t<T>, std::add_lvalue_reference_t<std::add_const_t<T>>>;

    template<class T>
    constexpr bool test_decl_view_v = std::is_same_v<decl_view_t<T>, typename T::view_type>;

    template<class T>
    constexpr auto view_is_same_decl_v = std::is_same_v<view_t<T>, decl_view_t<T>>;
}

void test_view() noexcept
{
    {
        using namespace private_detail_view;

        static_assert(is_small_size<bytes_array<small_size_v-1u>>::value);
        static_assert(is_small_size<bytes_array<small_size_v>>::value);
        static_assert(std::negation_v<is_small_size<bytes_array<small_size_v + 1u>>>);

        static_assert(is_small_size<bytes_array_with_copy_op<small_size_v - 1u>>::value);
        static_assert(is_small_size<bytes_array_with_copy_op<small_size_v>>::value);
        static_assert(std::negation_v<is_small_size<bytes_array_with_copy_op<small_size_v + 1u>>>);

        static_assert(is_view_by_copy<long double>::value);
        static_assert(is_view_by_copy<intmax_t>::value);
        static_assert(is_view_by_copy<uintmax_t>::value);

        static_assert(is_view_by_copy<bytes_array<small_size_v - 1u>>::value);
        static_assert(is_view_by_copy<bytes_array<small_size_v>>::value);
        static_assert(std::negation_v<is_view_by_copy<bytes_array<small_size_v + 1u>>>);

        static_assert(std::negation_v<is_view_by_copy<bytes_array_with_copy_op<small_size_v - 1u>>>);
        static_assert(std::negation_v<is_view_by_copy<bytes_array_with_copy_op<small_size_v>>>);
        static_assert(std::negation_v<is_view_by_copy<bytes_array_with_copy_op<small_size_v + 1u>>>);
    }

    {
        using private_detail_view::small_size_v;        

        static_assert(view_by_copy_is_same_v<long double>);
        static_assert(view_by_copy_is_same_v<intmax_t>);
        static_assert(view_by_copy_is_same_v<uintmax_t>);

        static_assert(view_by_copy_is_same_v<bytes_array<small_size_v - 1u>>);
        static_assert(view_by_copy_is_same_v<bytes_array<small_size_v>>);
        static_assert(view_by_copy_is_same_cref_v<bytes_array<small_size_v + 1u>>);

        static_assert(view_by_copy_is_same_v<bytes_array_with_view<small_size_v - 1u>>);
        static_assert(view_by_copy_is_same_v<bytes_array_with_view<small_size_v>>);
        static_assert(view_by_copy_is_same_cref_v<bytes_array_with_view<small_size_v + 1u>>);

        static_assert(view_by_copy_is_same_cref_v<bytes_array_with_copy_op<small_size_v - 1u>>);
        static_assert(view_by_copy_is_same_cref_v<bytes_array_with_copy_op<small_size_v>>);
        static_assert(view_by_copy_is_same_cref_v<bytes_array_with_copy_op<small_size_v + 1u>>);

        static_assert(view_by_copy_is_same_cref_v<bytes_array_with_copy_op_and_view<small_size_v - 1u>>);
        static_assert(view_by_copy_is_same_cref_v<bytes_array_with_copy_op_and_view<small_size_v>>);
        static_assert(view_by_copy_is_same_cref_v<bytes_array_with_copy_op_and_view<small_size_v + 1u>>);

        static_assert(view_is_same_v<long double>);
        static_assert(view_is_same_v<intmax_t>);
        static_assert(view_is_same_v<uintmax_t>);

        static_assert(view_is_same_v<bytes_array<small_size_v - 1u>>);
        static_assert(view_is_same_v<bytes_array<small_size_v>>);
        static_assert(view_is_same_cref_v<bytes_array<small_size_v + 1u>>);

        static_assert(test_decl_view_v<bytes_array_with_view<small_size_v - 1u>>);
        static_assert(test_decl_view_v<bytes_array_with_view<small_size_v>>);
        static_assert(test_decl_view_v<bytes_array_with_view<small_size_v + 1u>>);

        static_assert(view_is_same_decl_v<bytes_array_with_view<small_size_v - 1u>>);
        static_assert(view_is_same_decl_v<bytes_array_with_view<small_size_v>>);
        static_assert(view_is_same_decl_v<bytes_array_with_view<small_size_v + 1u>>);

        static_assert(view_is_same_cref_v<bytes_array_with_copy_op<small_size_v - 1u>>);
        static_assert(view_is_same_cref_v<bytes_array_with_copy_op<small_size_v>>);
        static_assert(view_is_same_cref_v<bytes_array_with_copy_op<small_size_v + 1u>>);

        static_assert(test_decl_view_v<bytes_array_with_copy_op_and_view<small_size_v - 1u>>);
        static_assert(test_decl_view_v<bytes_array_with_copy_op_and_view<small_size_v>>);
        static_assert(test_decl_view_v<bytes_array_with_copy_op_and_view<small_size_v + 1u>>);

        static_assert(view_is_same_decl_v<bytes_array_with_copy_op_and_view<small_size_v - 1u>>);
        static_assert(view_is_same_decl_v<bytes_array_with_copy_op_and_view<small_size_v>>);
        static_assert(view_is_same_decl_v<bytes_array_with_copy_op_and_view<small_size_v + 1u>>);

        static_assert(is_view_v<long double>);
        static_assert(is_view_v<intmax_t>);
        static_assert(is_view_v<uintmax_t>);

        static_assert(is_view_v<bytes_array<small_size_v - 1u>>);
        static_assert(is_view_v<bytes_array<small_size_v>>);
        static_assert(!is_view_v<bytes_array<small_size_v + 1u>>);

        static_assert(is_view_v<bytes_array_with_view<small_size_v - 1u>>);
        static_assert(is_view_v<bytes_array_with_view<small_size_v>>);
        static_assert(is_view_v<bytes_array_with_view<small_size_v + 1u>>);

        static_assert(!is_view_v<bytes_array_with_copy_op<small_size_v - 1u>>);
        static_assert(!is_view_v<bytes_array_with_copy_op<small_size_v>>);
        static_assert(!is_view_v<bytes_array_with_copy_op<small_size_v + 1u>>);

        static_assert(is_view_v<bytes_array_with_copy_op_and_view<small_size_v - 1u>>);
        static_assert(is_view_v<bytes_array_with_copy_op_and_view<small_size_v>>);
        static_assert(is_view_v<bytes_array_with_copy_op_and_view<small_size_v + 1u>>);
    }

    {
        struct s_test
        {
            uint32_t value;

            struct view_type
            {
                uint32_t value;
            };

            constexpr operator view_type () const noexcept
            {
                return { ~value };
            }
        };

        constexpr s_test value{ 0xfedccdefu };
        constexpr auto view_value = view(value);
        static_assert(std::is_same_v<std::remove_cv_t<decltype(view_value)>, s_test::view_type>);
        static_assert(0x01233210u == view_value.value);
    }

    D_ASSERT(!errno);
}