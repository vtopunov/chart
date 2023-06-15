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
}

void test_pixmap() noexcept
{
    using rgba_pixmap = pixmap<rgba_color32_t>;

    static_assert(std::is_same_v<rgba_pixmap::pixel_type, rgba_color32_t>);
    static_assert(std::is_same_v<rgba_pixmap::space_type, pix32space>);
    static_assert(rgba_pixmap::alignment == px::default_alignment);

    static_assert(!px::is_pixspan<rgba_pixmap>::value);
    static_assert(px::is_space_type<rgba_pixmap>::value);
    static_assert(is_data_pointer<rgba_pixmap>::value);
    static_assert(px::is_convertible_space<rgba_pixmap, pix32space>::value);
    static_assert(px::is_compatible_pixspacecontainer_v<rgba_pixmap, pix32space>);

    {
        rgba_pixmap rgba_pixmap;
        test_convert_to_pixspan<rgba_color32_t>(rgba_pixmap, rgba_pixmap);
    }

    {
        rgba_pixmap rgba_pixmap{ 2_px, 3_px };
        test_convert_to_pixspan<rgba_color32_t>(rgba_pixmap, rgba_pixmap);
    }

    {
        rgba_pixmap rgba_pixmap{ 3_px, 5_px };
        pixspan deduction_span{ rgba_pixmap };
        test_convert_to_pixspan(rgba_pixmap, deduction_span);
    }

    {
        const rgba_pixmap rgba_pixmap{ 3_px, 5_px };
        pixspan deduction_span{ rgba_pixmap };
        test_convert_to_pixspan(rgba_pixmap, deduction_span);
    }

    D_ASSERT(!errno);
}
