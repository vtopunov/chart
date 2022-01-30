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
    using decl_data_pointer_t = decltype(as_pointer(std::declval<T>().data()));
    
    template<class T>
    using decl_pixel_type_t = typename T::pixel_type;


    template<class T>
    inline constexpr auto decl_alignment_v = decl_space_type_t<T>::alignment;


    template<class T>
    using is_space_type = is_detected<decl_space_type_t, T>;

    template<class T>
    using is_data_pointer = is_detected<decl_data_pointer_t, T>;

    template<class T>
    using is_pixel_type = is_detected<decl_pixel_type_t, T>;

    
    template<class T, class Space>
    struct is_convertible_space : std::is_convertible<decl_space_type_t<T>, Space>
    {};

    template<class T, class Px>
    struct is_same_px : std::is_same<std::remove_const_t<decl_pixel_type_t<T>>, Px>
    {};

    
    template <class C, class Space>
    inline constexpr bool is_compatible_pixspacecontainer_v = std::conjunction_v
        <
        std::negation<is_pixspan<C>>,
        is_space_type<C>,
        is_data_pointer<C>,
        is_convertible_space<C, Space>
        >;

    template <class C, class Px>
    inline constexpr bool is_compatible_for_write_v = std::conjunction_v
        <
        is_space_type<C>,
        is_data_pointer<C>,
        is_pixel_type<C>,
        is_same_px<C, Px>
        >;

    
    template<class T>
    using decl_const_pixspan_t = pixspan<std::add_const_t<decl_pixel_type_t<T>>, decl_alignment_v<T>>;

    template<class T>
    constexpr decl_const_pixspan_t<T> as_const_pixspan(const T& image) noexcept
    {
        return image;
    }
    
    template<class T, size_t OutAlignment, size_t InAlignment>
    constexpr size2d_t write(pixspan<T, OutAlignment> in, point2d_t position, pixspan<const T, InAlignment> out) noexcept;

    template<class T, size_t Alignment = default_alignment>
    class pixspan : public pixspace<sizeof(T), Alignment>
    {
    public:
        using pixel_type = T;
        using space_type = pixspace<sizeof(pixel_type), Alignment>;
        using pixline_type = pixline<pixel_type>;

        using line_size_type = typename space_type::line_size_type;

        static constexpr auto alignment = space_type::alignment;
        static constexpr auto is_dynamic_alignment = space_type::is_dynamic_alignment;

        using const_pixel_type = std::add_const_t<pixel_type>;
        using mutable_pixel_type = std::remove_const_t<pixel_type>;
        static constexpr auto is_immutable = std::is_const_v<pixel_type>;
        static constexpr auto is_mutable = !is_immutable;

        using pointer = pixel_type*;
        using const_pointer = const_pixel_type*;

        using mutable_pixspan = pixspan<mutable_pixel_type, alignment>;

        constexpr pixspan() noexcept = default;

        constexpr pixspan(pointer data, space_type space) noexcept
            : space_type{ space }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, size2d_t sizes) noexcept
            : space_type{ sizes }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, pxside_t x, pxside_t y) noexcept
            : space_type{ x, y }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, size2d_t sizes, line_size_type line_size) noexcept
            : space_type{ sizes, line_size }
            , data_{ data }
        {}

        constexpr pixspan(pointer data, pxside_t x, pxside_t y, line_size_type line_size) noexcept
            : space_type{ x, y, line_size }
            , data_{ data }
        {}

        constexpr pixspan(const pixspan&) noexcept = default;

        template<bool dummy = true, class = std::enable_if_t<(dummy) && is_immutable>>
        constexpr pixspan(const mutable_pixspan& span) noexcept
            : space_type{ span }
            , data_{ span.data() }
        {}

        template<size_t Align, class = std::enable_if_t<(is_dynamic_alignment) && (!is_dynamic_alignment_v<Align>)>>
        constexpr pixspan(const pixspan<const_pixel_type, Align>& span) noexcept
            : space_type{ span }
            , data_{ span.data() }
        {}

        template<size_t Align, class = std::enable_if_t<(is_immutable) && (is_dynamic_alignment) && (!is_dynamic_alignment_v<Align>)>>
        constexpr pixspan(const pixspan<mutable_pixel_type, Align>& span) noexcept
            : space_type{ span }
            , data_{ span.data() }
        {}

        template<class C, class = std::enable_if_t<is_compatible_pixspacecontainer_v<C, space_type>>>
        constexpr pixspan(C& container) noexcept
            : space_type{ space(container) }
            , data_{ as_pointer(std::data(container)) }
        {}

        constexpr pixspan& operator = (const pixspan&) noexcept = default;

        template<bool dummy = true, class = std::enable_if_t<(dummy) && is_immutable>>
        constexpr pixspan& operator = (const mutable_pixspan& span) noexcept
        {
            space_type::operator = (span);
            data_ = span.data_;
            return *this;
        }

        template<size_t Align, class = std::enable_if_t<(is_dynamic_alignment) && (!is_dynamic_alignment_v<Align>)>>
        constexpr pixspan& operator = (const pixspan<const_pixel_type, Align>& span) noexcept
        {
            space_type::operator = (span);
            data_ = span.data_;
            return *this;
        }


        template<size_t Align, class = std::enable_if_t<(is_immutable) && (is_dynamic_alignment) && (!is_dynamic_alignment_v<Align>)>>
        constexpr pixspan& operator = (const pixspan<mutable_pixel_type, Align>& span) noexcept
        {
            space_type::operator = (span);
            data_ = span.data_;
            return *this;
        }

        [[nodiscard]]
        constexpr pointer data() const noexcept
        {
            return data_;
        }

        constexpr pixline_type line0() const noexcept
        {
            return { data_, space_type::line_size() };
        }

        constexpr pixline_type begin() const noexcept
        {
            return line0();
        }

        constexpr const_pointer end() const noexcept
        {
            return data_ + space_type::size();
        }

        template<class T>
        static constexpr bool is_compatible_for_store = (is_mutable) && is_compatible_for_write_v<T, pixel_type>;

        template<class T>
        constexpr std::enable_if_t<is_compatible_for_store<T>, size2d_t> store(point2d_t position, const T& image) const noexcept
        {
            return write(*this, position, as_const_pixspan(image));
        }

        template<class T>
        constexpr std::enable_if_t<is_compatible_for_store<T>, size2d_t> store(pxside_t x, pxside_t y, const T& image) const noexcept
        {
            return store(point2d_t{ x, y }, image);
        }

        template<class T>
        constexpr std::enable_if_t<is_compatible_for_store<T>, size2d_t> store(const T& image) const noexcept
        {
            return store(0_px, 0_px, image);
        }

    private:
        pointer data_{ nullptr };
    };

    template<class T, size_t OutAlignment, size_t InAlignment>
    constexpr size2d_t write(pixspan<T, OutAlignment> out, point2d_t position, pixspan<const T, InAlignment> in) noexcept
    {
        const auto x = std::min(position.x(), out.width());
        const auto y = std::min(position.y(), out.height());

        const size2d_t crop_sizes
        {
            std::min(in.width(), out.width() - x),
            std::min(in.height(), out.height() - y)
        };

        {
            const auto out_line_size = out.line_size();
            auto p_out = out.data() + y * out_line_size + x;

            for (const auto p_in : in)
            {
                std::copy_n(p_in, crop_sizes.width(), p_out);
                p_out += out_line_size;
            }
        }

        return crop_sizes;
    }


    template <class C>
    pixspan(C&)->pixspan<typename C::pixel_type, C::alignment>;

    template <class C>
    pixspan(const C&)->pixspan<const typename C::pixel_type, C::alignment>;


    using pix8span_t = pixspan<u8tint_t>;
    using const_pix8span_t = pixspan<const u8tint_t>;
    static_assert(std::is_same_v<pix8span_t::space_type, pix8space_t>);
}

using px::pixspan;
using px::pix8span_t;
using px::const_pix8span_t;