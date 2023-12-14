#pragma once

#include <px/pixspace.h>
#include <px/pixline.h>


namespace px
{
    template<class T, size_t A>
    class pixspan;

    template <class T>
    struct is_pixspan : std::false_type
    {};

    template <class T, size_t A>
    struct is_pixspan<pixspan<T, A>> : std::true_type
    {};

    template <class T>
    struct is_pixspan<const T> : is_pixspan<T>
    {};


    template<class T>
    using decl_space_type_t = typename T::space_type;

    template<class T>
    using decl_pixel_type_t = typename T::pixel_type;


    template<class T>
    constexpr auto decl_alignment_v = decl_space_type_t<T>::alignment;

    template<class T>
    using is_space_type = is_detected<decl_space_type_t, T>;

    template<class T>
    using is_pixel_type = is_detected<decl_pixel_type_t, T>;

    template<class T, class Space>
    struct is_convertible_space : std::is_convertible<decl_space_type_t<T>, Space>
    {};

    template<class T, class Px>
    struct is_same_px : std::is_same<std::remove_const_t<decl_pixel_type_t<T>>, Px>
    {};


    template <class C, class Space>
    constexpr bool is_compatible_pixspacecontainer_v = std::conjunction_v
        <
        std::negation<is_pixspan<C>>,
        is_space_type<C>,
        is_data_pointer<C>,
        is_convertible_space<C, Space>
        >;

    template <class C, class Px>
    constexpr bool is_compatible_for_write_v = std::conjunction_v
        <
        is_space_type<C>,
        is_data_pointer<C>,
        is_pixel_type<C>,
        is_same_px<C, Px>,
        std::negation<std::is_const<Px>>
        >;



    template<class TestT, size_t TestAlign, class BaseT, size_t BaseAling>
    constexpr bool is_compatible_pixspan_v = std::conjunction_v
    <
        std::negation<std::conjunction<std::is_same<TestT, BaseT>, std::bool_constant<TestAlign == BaseAling>>>,
        std::is_same<copy_const_t<BaseT, TestT>, BaseT>,
        std::disjunction<is_dynamic_alignment<BaseAling>, std::bool_constant<TestAlign == BaseAling>>
    >;
    
    template<class T>
    using decl_const_pixspan_t = pixspan<std::add_const_t<decl_pixel_type_t<T>>, decl_alignment_v<T>>;

    template<class T> [[nodiscard]]
    constexpr decl_const_pixspan_t<T> as_const_pixspan(const T& image) noexcept
    {
        return image;
    }

    template<class T, size_t OutAlignment, size_t InAlignment>
    constexpr pxsize2d write(pixspan<T, OutAlignment> in, pxpoint2d position, pixspan<const T, InAlignment> out) noexcept;


    template<class T, size_t Alignment = default_alignment>
    class pixspan : public pixspace<sizeof(T), Alignment>
    {
    public:
        using pixel_type = T;
        static constexpr auto alignment = Alignment;
        using space_type = pixspace<sizeof(pixel_type), alignment>;
        using pixline_type = pixline<pixel_type>;
        using line_size_type = typename space_type::line_size_type;

        using pointer = pixel_type*;
        using const_pointer = const pixel_type*;

        constexpr pixspan() noexcept = default;

        constexpr pixspan(pointer data, space_type space) noexcept
            : space_type{ space }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, pxsize2d sizes) noexcept
            : space_type{ sizes }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, pxside_t w, pxside_t h) noexcept
            : space_type{ w, h }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, pxsize2d sizes, line_size_type line_size) noexcept
            : space_type{ sizes, line_size }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, pxside_t w, pxside_t h, line_size_type line_size) noexcept
            : space_type{ w, h, line_size }
            , data_{ data }
        {}

        constexpr pixspan(const pixspan&) noexcept = default;

        template<class TestPxType, size_t TestAlign>
        static constexpr bool is_compatible_pixspan_v = px::is_compatible_pixspan_v<TestPxType, TestAlign, pixel_type, alignment>;

        template<class OtherPxType, size_t Align, std::enable_if_t<is_compatible_pixspan_v<OtherPxType, Align>, int> = 0>
        constexpr pixspan(const pixspan<OtherPxType, Align>& span) noexcept
            : space_type{ span }
            , data_{ span.data() }
        {}

        template<class C, std::enable_if_t<is_compatible_pixspacecontainer_v<C, space_type>, int> = 0>
        constexpr pixspan(C& container) noexcept
            : space_type{ space(container) }
            , data_{ as_pointer(std::data(container)) }
        {}

        constexpr pixspan& operator = (const pixspan&) noexcept = default;

        template<class OtherPxType, size_t Align>
        constexpr std::enable_if_t <is_compatible_pixspan_v<OtherPxType, Align>, pixspan&> operator = (const pixspan<OtherPxType, Align>& span) noexcept
        {
            space_type::operator = (span);
            data_ = span.data();
            return *this;
        }

        [[nodiscard]]
        constexpr pointer operator [] (size_t index) const noexcept
        {
            return data_ + index * space_type::line_size();
        }

        [[nodiscard]]
        constexpr pointer data() const noexcept
        {
            return data_;
        }

        [[nodiscard]]
        constexpr pixline_type line0() const noexcept
        {
            return { data_, space_type::line_size() };
        }

        [[nodiscard]]
        constexpr pixline_type begin() const noexcept
        {
            return line0();
        }

        [[nodiscard]]
        constexpr const_pointer end() const noexcept
        {
            return data_ + space_type::size();
        }

        template<class C>
        static constexpr bool is_compatible_for_store_v = is_compatible_for_write_v<C, pixel_type>;

        template<class C>
        constexpr std::enable_if_t<is_compatible_for_store_v<C>, pxsize2d> store(pxpoint2d position, const C& image) const noexcept
        {
            return write(*this, position, as_const_pixspan(image));
        }

        template<class C>
        constexpr std::enable_if_t<is_compatible_for_store_v<C>, pxsize2d> store(pxside_t x, pxside_t y, const C& image) const noexcept
        {
            return store(point2d{ x, y }, image);
        }

        template<class C>
        constexpr std::enable_if_t<is_compatible_for_store_v<C>, pxsize2d> store(const C& image) const noexcept
        {
            return store(0_px, 0_px, image);
        }

    private:
        pointer data_{ nullptr };
    };

    template<class T, size_t OutAlignment, size_t InAlignment>
    constexpr pxsize2d write(pixspan<T, OutAlignment> out, pxpoint2d position, pixspan<const T, InAlignment> in) noexcept
    {
        const auto x = std::min(position.x(), out.width());
        const auto y = std::min(position.y(), out.height());

        const pxsize2d crop_sizes
        {
            std::min(in.width(), out.width() - x),
            std::min(in.height(), out.height() - y)
        };

        {
            const auto in_line_size = in.line_size();
            auto p_in = in.data();
            const auto in_end = in.data() + crop_sizes.height() * in_line_size;

            const auto out_line_size = out.line_size();
            auto p_out = out.data() + y * out_line_size + x;

            for (; p_in != in_end; p_in += in_line_size, p_out += out_line_size)
            {
                std::copy_n(p_in, crop_sizes.width(), p_out);
            }
        }

        return crop_sizes;
    }


    template <class C>
    pixspan(C&)->pixspan<typename C::pixel_type, C::alignment>;

    template <class C>
    pixspan(const C&)->pixspan<const typename C::pixel_type, C::alignment>;


    using pix8span = pixspan<u8tint_t>;
    using const_pix8span = pixspan<const u8tint_t>;
    static_assert(std::is_same_v<pix8span::space_type, pix8space>);
}

using px::pixspan;
using px::pix8span;
using px::const_pix8span;