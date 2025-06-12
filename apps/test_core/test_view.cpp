#include "core/view.h"


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

    struct test_buffer
    {
        const void* data() const { return nullptr; }
        size_t size() const { return {}; }
    };

    template<class T>
    using is_view_by_copy = std::bool_constant<private_detail_view::is_view_by_copy_v<T>>;

    template<class T>
    using is_qualified = std::disjunction<
        std::is_reference<T>,
        std::is_pointer<T>,
        std::is_const<T>,
        std::is_volatile<T>
    >;

    template<class T>
    constexpr auto test_view_by_copy_v = std::conjunction_v<
        std::negation<is_qualified<T>>,
        is_view_by_copy<T>,
        std::is_same<view_by_copy_t<T>, std::add_const_t<T>>,
        std::is_same<view_t<T>, std::add_const_t<T>>
    >;

    template<class T>
    constexpr auto test_view_by_cref_v = std::conjunction_v<
        std::negation<is_qualified<T>>,
        std::negation<is_view_by_copy<T>>,
        std::is_same<view_by_copy_t<T>, std::add_lvalue_reference_t<std::add_const_t<T>>>,
        std::is_same<view_t<T>, std::add_lvalue_reference_t<std::add_const_t<T>>>
    >;

    template<class T>
    constexpr auto test_view_with_copy_v = std::conjunction_v<
        std::negation<is_qualified<T>>,
        is_view_by_copy<T>,
        std::is_same<view_by_copy_t<T>, std::add_const_t<T>>,
        std::is_same<decl_view_type_t<T>, typename T::view_type>,
        std::is_same<view_t<T>, std::add_const_t<typename T::view_type>>
    >;

    template<class T>
    constexpr auto test_view_with_cref_v = std::conjunction_v<
        std::negation<is_qualified<T>>,
        std::negation<is_view_by_copy<T>>,
        std::is_same<view_by_copy_t<T>, std::add_lvalue_reference_t<std::add_const_t<T>>>,
        std::is_same<decl_view_type_t<T>, typename T::view_type>,
        std::is_same<view_t<T>, std::add_const_t<typename T::view_type>>
    >;

    template<class T>
    constexpr auto test_spanview_with_copy_v = std::conjunction_v<
        std::negation<is_qualified<T>>,
        is_view_by_copy<T>,
        std::is_same<view_by_copy_t<T>, std::add_const_t<T>>,
        std::negation<is_detected<decl_view_type_t, T>>,
        std::is_same<view_t<T>, std::add_const_t<span<std::add_const_t<value_type_t<T>>, extent_v<T>> > >
    >;

    template<class T>
    constexpr auto test_spanview_with_cref_v = std::conjunction_v<
        std::negation<is_qualified<T>>,
        std::negation<is_view_by_copy<T>>,
        std::is_same<view_by_copy_t<T>, std::add_lvalue_reference_t<std::add_const_t<T>>>,
        std::negation<is_detected<decl_view_type_t, T>>,
        std::is_same<view_t<T>, std::add_const_t<span<std::add_const_t<value_type_t<T>>, extent_v<T>> > >
    >;

    template<class T>
    constexpr auto test_bufferview_with_copy_v = std::conjunction_v<
        std::negation<is_qualified<T>>,
        is_view_by_copy<T>,
        std::is_same<view_by_copy_t<T>, std::add_const_t<T>>,
        std::negation<is_detected<decl_view_type_t, T>>,
        std::is_same<view_t<T>, std::add_const_t<const_buffer_view> >
    >;
}

void test_view() noexcept
{
    {
        static_assert(std::is_same_v<view_t<char*>, char* const>);
        static_assert(std::is_same_v<view_t<const char*>, const char*const>);
        static_assert(std::is_same_v<view_t<const char*const>, const char*const>);
        static_assert(std::is_same_v<view_t<std::byte*>, std::byte*const>);
        static_assert(std::is_same_v<view_t<const std::byte*>, const std::byte*const>);
        static_assert(std::is_same_v<view_t<const std::byte*const>, const std::byte*const>);
    }

    {
        using namespace private_detail_view;

        static_assert(is_small_size<bytes_array<small_size_mini - 1u>>::value);
        static_assert(is_small_size<bytes_array<small_size_mini>>::value);
        static_assert(std::negation_v<is_small_size<bytes_array<small_size_mini + 1u>>>);

        static_assert(is_small_size<bytes_array_with_copy_op<small_size_mini - 1u>>::value);
        static_assert(is_small_size<bytes_array_with_copy_op<small_size_mini>>::value);
        static_assert(std::negation_v<is_small_size<bytes_array_with_copy_op<small_size_mini + 1u>>>);

        static_assert(is_small_size<std::array<std::byte, small_size_mini - 1u>>::value);
        static_assert(is_small_size<std::array<std::byte, small_size_mini>>::value);
        static_assert(std::negation_v<is_small_size<std::array<std::byte, small_size_mini + 1u>>>);

        static_assert(is_view_by_copy_v<long double>);
        static_assert(is_view_by_copy_v<intmax_t>);
        static_assert(is_view_by_copy_v<uintmax_t>);

        static_assert(is_view_by_copy_v<bytes_array<small_size_mini - 1u>>);
        static_assert(is_view_by_copy_v<bytes_array<small_size_mini>>);
        static_assert(std::negation_v<is_view_by_copy<bytes_array<small_size_mini + 1u>>>);

        static_assert(is_view_by_copy_v<bytes_array<small_size_mini - 1u>>);
        static_assert(is_view_by_copy_v<bytes_array<small_size_mini>>);
        static_assert(std::negation_v<is_view_by_copy<bytes_array<small_size_mini + 1u>>>);

        static_assert(std::negation_v<is_view_by_copy<bytes_array_with_copy_op<small_size_mini - 1u>>>);
        static_assert(std::negation_v<is_view_by_copy<bytes_array_with_copy_op<small_size_mini>>>);
        static_assert(std::negation_v<is_view_by_copy<bytes_array_with_copy_op<small_size_mini + 1u>>>);

        static_assert(is_view_by_copy_v<std::array<std::byte, small_size_mini - 1u>>);
        static_assert(is_view_by_copy_v<std::array<std::byte, small_size_mini>>);
        static_assert(std::negation_v<is_view_by_copy<std::array<std::byte, small_size_mini + 1u>>>);
    }

    {
        static_assert(test_view_by_copy_v<long double>);
        static_assert(test_view_by_copy_v<intmax_t>);
        static_assert(test_view_by_copy_v<uintmax_t>);

        static_assert(test_view_by_copy_v<bytes_array<small_size_mini - 1u>>);
        static_assert(test_view_by_copy_v<bytes_array<small_size_mini>>);
        static_assert(test_view_by_cref_v<bytes_array<small_size_mini + 1u>>);

        static_assert(test_view_with_copy_v<bytes_array_with_view<small_size_mini - 1u>>);
        static_assert(test_view_with_copy_v<bytes_array_with_view<small_size_mini>>);
        static_assert(test_view_with_cref_v<bytes_array_with_view<small_size_mini + 1u>>);

        static_assert(test_view_by_cref_v<bytes_array_with_copy_op<small_size_mini - 1u>>);
        static_assert(test_view_by_cref_v<bytes_array_with_copy_op<small_size_mini>>);
        static_assert(test_view_by_cref_v<bytes_array_with_copy_op<small_size_mini + 1u>>);

        static_assert(test_view_with_cref_v<bytes_array_with_copy_op_and_view<small_size_mini - 1u>>);
        static_assert(test_view_with_cref_v<bytes_array_with_copy_op_and_view<small_size_mini>>);
        static_assert(test_view_with_cref_v<bytes_array_with_copy_op_and_view<small_size_mini + 1u>>);

        static_assert(test_spanview_with_copy_v<std::array<std::byte, small_size_mini - 1u>>);
        static_assert(test_spanview_with_copy_v<std::array<std::byte, small_size_mini>>);
        static_assert(test_spanview_with_cref_v<std::array<std::byte, small_size_mini + 1>>);

        static_assert(test_bufferview_with_copy_v<test_buffer>);
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