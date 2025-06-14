#pragma once

#include <px/pixspace.h>
#include <px/pixline.h>


namespace px
{
    template<class T, size_t Alignment>
    class pixspan;

    template <class T>
    struct is_pixspan : std::false_type
    {};

    template <class T, size_t Alignment>
    struct is_pixspan<pixspan<T, Alignment>> : std::true_type
    {};

    template <class T>
    struct is_pixspan<const T> : is_pixspan<T>
    {};


    template<class T>
    using decl_pixel_type_t = typename T::pixel_type;

    template<class T>
    using decl_space_t = std::remove_cvref_t<decltype(space(std::declval<const T&>()))>;

    template<class T>
    constexpr auto decl_alignment_v = T::alignment;

    template<class T>
    constexpr auto decl_space_alignment_v = decl_alignment_v<decl_space_t<T>>;

    template<class T>
    using decl_space_alignment_t = decltype(decl_space_alignment_v<T>);

    template<class T>
    using has_space_alignment = is_detected<decl_space_alignment_t, T>;

    template<class C, class ToSpace>
    using is_pixcontainer_space_convertible = is_detected_convertible<ToSpace, decl_space_t, C>;


    template<class OtherContainer, class Px, class Space>
    using is_compatible_pixcontainer = std::conjunction<
        std::negation<is_pixspan<OtherContainer>>,
        has_std_data_compatible<Px, OtherContainer>,
        is_pixcontainer_space_convertible<OtherContainer, Space>
    >;

    template <class OtherContainer, class Px, class Space>
    constexpr bool is_compatible_pixcontainer_v = is_compatible_pixcontainer<OtherContainer, Px, Space>::value;


    template<class Px0, size_t Align0, class Px1, size_t Align1>
    using is_equal_pixspan_argument = std::conjunction<std::is_same<Px0, Px1>, alignment_is_equal<Align0, Align1>>;

    template<size_t OtherAlign, size_t Align>
    using is_compatible_alignment = std::disjunction<is_dynamic_alignment<Align>, alignment_is_equal<Align, OtherAlign>>;

    template<class OtherPx, size_t OtherAlign, class Px, size_t Align>
    constexpr bool is_compatible_pixspan_v = std::conjunction_v<
        std::negation<is_equal_pixspan_argument<OtherPx, OtherAlign, Px, Align>>,
        is_const_convertible<OtherPx, Px>,
        is_compatible_alignment<OtherAlign, Align>
    >;


    template<class T>
    using decl_remove_const_std_data_value_t = std::remove_const_t<decl_std_data_value_t<T>>;

    template <class InputContainer, class OutputPx>
    using is_std_data_compatible_for_write = is_detected_exact<OutputPx, decl_remove_const_std_data_value_t, InputContainer>;

    template <class InputContainer, class OutputPx>
    constexpr bool is_compatible_for_write_v = std::conjunction_v<
        std::negation<std::is_const<OutputPx>>,
        is_std_data_compatible_for_write<InputContainer, OutputPx>,
        has_space_alignment<InputContainer>
    >;


    template<class T, size_t OutputAlignment, size_t InputAlignment>
    constexpr pxsizes write(pixspan<T, OutputAlignment> output, pxpoint position, pixspan<const T, InputAlignment> input) noexcept;


    template<class T, size_t Alignment = default_alignment>
    class pixspan : public pixspace<sizeof(T), Alignment>
    {
    public:
        using pixel_type = T;
        static constexpr auto alignment = Alignment;
        using space_type = pixspace<sizeof(pixel_type), alignment>;
        using line_type = pixline<pixel_type>;
        using line_span_type = typename line_type::span_type;
        using line_size_type = typename space_type::line_size_type;
        using const_pixel_type = const pixel_type;
        using pointer = pixel_type*;
        using const_pointer = const_pixel_type*;

        template<class OtherContainer>
        static constexpr bool is_compatible_container_v = is_compatible_pixcontainer_v<OtherContainer, pixel_type, space_type>;

        template<class OtherPx, size_t OtherAlign>
        static constexpr bool is_compatible_span_v = is_compatible_pixspan_v<OtherPx, OtherAlign, pixel_type, alignment>;

        template<class OtherContainer>
        static constexpr bool is_compatible_for_store_v = is_compatible_for_write_v<OtherContainer, pixel_type>;

        constexpr pixspan() noexcept = default;

        constexpr pixspan(pointer data, space_type space) noexcept
            : space_type{ space }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, npx_t w, npx_t h) noexcept
            : space_type{ w, h }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, pxsizes sizes, line_size_type line_size) noexcept
            : space_type{ sizes, line_size }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, npx_t w, npx_t h, line_size_type line_size) noexcept
            : space_type{ w, h, line_size }
            , data_{ data }
        {}

        constexpr pixspan(const pixspan&) noexcept = default;

        template<class OtherPx, size_t OtherAlign, std::enable_if_t<is_compatible_span_v<OtherPx, OtherAlign>, int> = 0>
        constexpr pixspan(const pixspan<OtherPx, OtherAlign>& span) noexcept
            : space_type{ span }
            , data_{ span.data() }
        {}

        template<class C, std::enable_if_t<is_compatible_container_v<C>, int> = 0>
        constexpr pixspan(C& container) noexcept
            : space_type{ space(container) }
            , data_{ std::data(container) }
        {}

        constexpr pixspan& operator = (const pixspan&) noexcept = default;

        template<class OtherPx, size_t OtherAlign>
        constexpr std::enable_if_t <is_compatible_span_v<OtherPx, OtherAlign>, pixspan&> operator = (const pixspan<OtherPx, OtherAlign>& span) noexcept
        {
            space_type::operator = (span);
            data_ = span.data();
            return *this;
        }

        [[nodiscard]]
        constexpr pointer data() const noexcept
        {
            return data_;
        }

        [[nodiscard]]
        constexpr const const_pointer cdata() const noexcept
        {
            return data_;
        }

        [[nodiscard]]
        constexpr line_type lines() const noexcept
        {
            return { data_, space_type::line_size(), space_type::width() };
        }

        [[nodiscard]]
        constexpr line_type lines(npx_t index) const noexcept
        {
            D_ASSERT_OR_ASSUME(index < space_type::height());
            const auto line_size = space_type::line_size();
            return { data_ + index * line_size, line_size, space_type::width() };
        }

        [[nodiscard]]
        constexpr line_span_type line() const noexcept
        {
            return lines().pixels();
        }

        [[nodiscard]]
        constexpr line_span_type line(npx_t index) const noexcept
        {
            return lines(index).pixels();
        }

        [[nodiscard]]
        constexpr line_type begin() const noexcept
        {
            return lines();
        }

        [[nodiscard]]
        constexpr const_pointer end() const noexcept
        {
            return data_ + space_type::size();
        }

        template<class C>
        constexpr std::enable_if_t<is_compatible_for_store_v<C>, pxsizes> store(pxpoint position, const C& image) const noexcept
        {
            return _store(position, image);
        }

        template<class C>
        constexpr std::enable_if_t<is_compatible_for_store_v<C>, pxsizes> store(npx_t x, npx_t y, const C& image) const noexcept
        {
            return _store(pxpoint{ x, y }, image);
        }

        template<class C>
        constexpr std::enable_if_t<is_compatible_for_store_v<C>, pxsizes> store(const C& image) const noexcept
        {
            return _store(pxpoint{ 0_npx, 0_npx }, image);
        }

    private:
        template<class C>
        constexpr pxsizes _store(pxpoint position, const C& image) const noexcept
        {
            static_assert(is_compatible_for_store_v<C>);
            const auto image_space = space(image);
            constexpr auto align = decl_alignment_v<decltype(image_space)>;
            const pixspan<const pixel_type, align> span_image{ ::cdata(image), image_space };
            return px::write(*this, position, span_image);
        }

    private:
        pointer data_{ nullptr };
    };

    template <class C>
    pixspan(C&) -> pixspan<decl_pixel_type_t<C>, decl_alignment_v<C>>;

    template <class C>
    pixspan(const C&) -> pixspan<std::add_const_t<decl_pixel_type_t<C>>, decl_alignment_v<C>>;

    template<class T, size_t Alignment>
    pixspan(T*, pixspace<sizeof(T), Alignment>) -> pixspan<T, Alignment>;


    template<class T, size_t OutputAlignment, size_t InputAlignment>
    constexpr pxsizes write(pixspan<T, OutputAlignment> output, pxpoint position, pixspan<const T, InputAlignment> input) noexcept
    {
        const auto x = std::min(position.x(), output.width());
        const auto y = std::min(position.y(), output.height());

        const pxsizes crop_sizes
        {
            std::min(input.width(), output.width() - x),
            std::min(input.height(), output.height() - y)
        };

        {
            const auto input_line_size = input.line_size();
            auto p_input = input.data();
            const auto input_end = input.data() + crop_sizes.height() * input_line_size;

            const auto output_line_size = output.line_size();
            auto p_output = output.data() + y * output_line_size + x;

            for (; p_input != input_end; p_input += input_line_size, p_output += output_line_size)
            {
                std::copy_n(p_input, crop_sizes.width(), p_output);
            }
        }

        return crop_sizes;
    }


    using lumpixspan = pixspan<luminance_t>;
    using const_lumpixspan = pixspan<const luminance_t>;
    using rgba_color_pixspan = pixspan<rgba_color>;
    using const_rgba_color_pixspan = pixspan<const rgba_color>;

    static_assert(std::is_same_v<lumpixspan::space_type, luminance_pixspace>);
}

using px::pixspan;
using px::lumpixspan;
using px::const_lumpixspan;
using px::rgba_color_pixspan;
using px::const_rgba_color_pixspan;