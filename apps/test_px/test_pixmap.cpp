#include <core/color.h>

#include <px/pixmap.h>


namespace
{
    template<class T>
    void test_convert_to_pixspan(pixmap<T>& map, pixspan<T> span) noexcept
    {
        using pixmap_type = pixmap<T>;
        using pixspan_type = pixspan<T>;

        static_assert(std::is_same_v<typename pixmap_type::pixel_type, typename pixspan_type::pixel_type>);
        static_assert(pixmap_type::alignment == pixspan_type::alignment);
        static_assert(std::is_same_v<typename pixmap_type::space_type, typename pixspan_type::space_type>);
        static_assert(std::is_same_v<decltype(map.data()), decltype(span.data())>);
        static_assert(std::is_same_v<decltype(map.sizes()), decltype(span.sizes())>);

        D_ASSERT(map.data() == span.data());
        D_ASSERT(map.sizes() == span.sizes());
    }

    template<class T>
    void test_convert_to_pixspan(const pixmap<T>& map, pixspan<const T> span) noexcept
    {
        pixspan<T> mutable_span{ const_cast<T*>(std::data(span)), px::space(span) };
        test_convert_to_pixspan(as_mutable(map), mutable_span);
    }

    template<class T>
    using has_space_type = is_detected<px::decl_space_t, T>;

    template<class T>
    using has_pixel_type = is_detected<px::decl_pixel_type_t, T>;

    template<class T, class TestPx, class TestSpace>
    constexpr bool test_types_v = std::conjunction_v<
        has_pixel_type<T>,
        has_space_type<T>,
        px::has_space_alignment<T>, 
        is_std_data_convertible<T, TestPx*>,
        px::is_pixcontainer_space_convertible<T, TestSpace>,
        std::is_same<px::decl_pixel_type_t<T>, TestPx>,
        std::is_same<px::decl_space_t<T>, TestSpace>,
        px::alignment_is_equal<px::decl_alignment_v<T>, px::default_alignment>,
        std::disjunction<px::is_pixspan<T>, px::is_compatible_pixcontainer<T, TestPx, TestSpace>>
    >;

    template<class T>
    constexpr bool test_rgba_color_types_v = test_types_v<T, rgba_color, rgba_pixspace>;

    template<class T>
    constexpr bool test_const_rgba_color_types_v = test_types_v<T, const rgba_color, rgba_pixspace>;
}

void test_pixmap() noexcept
{
    static_assert(test_rgba_color_types_v<rgba_color_pixspan>);
    static_assert(test_const_rgba_color_types_v<const_rgba_color_pixspan>);
    static_assert(test_rgba_color_types_v<rgba_color_pixmap>);

    {
        rgba_color_pixmap m;
        test_convert_to_pixspan<rgba_color>(m, m);
    }

    {
        rgba_color_pixmap m{ 2_npx, 3_npx };
        test_convert_to_pixspan<rgba_color>(m, m);
    }

    {
        rgba_color_pixmap m{ 3_npx, 5_npx };
        pixspan mspan{ m };
        static_assert(std::is_same_v<decltype(mspan), rgba_color_pixspan>);
        test_convert_to_pixspan(m, mspan);
    }


    {
        const rgba_color_pixmap m{ 3_npx, 5_npx };
        pixspan mspan{ m };
        static_assert(std::is_same_v<decltype(mspan), const_rgba_color_pixspan>);
        test_convert_to_pixspan(m, mspan);
    }

    D_ASSERT(!errno);
}
