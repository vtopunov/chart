#pragma once

#include <core/buffer.h>
#include <core/buffer_view.h>

#include <px/pixspan.h>

namespace px
{
    struct pixmap_construct_t
    {};

    constexpr pixmap_construct_t pixmap_construct{};

    template<class T, size_t Alignment = default_alignment>
    class pixmap : public pixspace<sizeof(T), Alignment>
    {
    public:
        using pixel_type = T;
        using const_pixel_type = std::add_const_t<T>;
        using space_type = pixspace<sizeof(T), Alignment>;
        using span_type = pixspan<T, Alignment>;
        using const_span_type = pixspan<const_pixel_type, Alignment>;
        using pixline_type = typename span_type::pixline_type;
        using const_pixline_type = typename const_span_type::pixline_type;
        using view_type = span_type;
        using const_view_type = pixspan<const_pixel_type, Alignment>;
        using line_size_type = typename space_type::line_size_type;
        
        using pointer = pixel_type*;
        using const_pointer = const_pixel_type*;

        constexpr pixmap() noexcept = default;
        
        constexpr pixmap(const pixmap&) noexcept = delete;

        constexpr pixmap(pixmap_construct_t, buffer_t& buffer, const space_type& space) noexcept
            : space_type{ space }
            , buffer_{ std::move(buffer) }
        {
            D_ASSERT(space.size_bytes() <= buffer_.size());
        }

        constexpr pixmap(pixmap&& right) noexcept 
            : space_type{ std::exchange(right._space_ref(), {}) }
            , buffer_{ std::move(right.buffer_) }
        {}

        explicit pixmap(const space_type& space) noexcept
            : space_type{ space }
            , buffer_{ buffer_construct, space.size_bytes() }
        {
            if (buffer_)
            {
                zero_memory(*this);
            }
            else
            {
                _reject_space();
            }
        }

        pixmap(buffer_t buffer, const space_type& space) noexcept
            : space_type{ space }
            , buffer_{ std::move(buffer) }
        {
            if (buffer_.try_resize(space.size_bytes()))
            {
                zero_memory(*this);
            }
            else
            {
                _reject_space();
            }
        }

        explicit pixmap(buffer_t buffer) noexcept
            : pixmap{ std::move(buffer), space_type{} }
        {}

        explicit pixmap(size2d sizes) noexcept
            : pixmap{ space_type{ sizes } }
        {}

        pixmap(buffer_t buffer, size2d sizes) noexcept
            : pixmap{ std::move(buffer), space_type{ sizes } }
        {}

        pixmap(pxside_t x, pxside_t y) noexcept
            : pixmap{ space_type{ x, y } }
        {}

        pixmap(buffer_t buffer, pxside_t x, pxside_t y) noexcept
            : pixmap{ std::move(buffer), space_type{ x, y } }
        {}

        pixmap(size2d sizes, line_size_type line_size) noexcept
            : pixmap{ space_type{ sizes, line_size } }
        {}

        pixmap(buffer_t buffer, size2d sizes, line_size_type line_size) noexcept
            : pixmap{ std::move(buffer), space_type{ sizes, line_size } }
        {}

        pixmap(pxside_t x, pxside_t y, line_size_type line_size) noexcept
            : pixmap{ space_type{ x, y, line_size } }
        {}

        pixmap(buffer_t buffer, pxside_t x, pxside_t y, line_size_type line_size) noexcept
            : pixmap{ std::move(buffer), space_type{ x, y, line_size } }
        {}

        constexpr pixmap& operator = (const pixmap&) noexcept = delete;

        constexpr pixmap& operator = (pixmap&& right) noexcept
        {
            swap(right);
            return *this;
        }

        constexpr explicit operator bool() const noexcept
        {
            return !!buffer_;
        }

        constexpr void swap(pixmap& right) noexcept
        {
            std::swap(_space_ref(), right._space_ref());
            buffer_.swap(right.buffer_);
        }

        constexpr void swap(buffer_t& right) noexcept
        {
            _reject_space();
            buffer_.swap(right);
        }

        [[nodiscard]]
        constexpr pointer data() noexcept
        {
            return buffer_.as_ptr<pixel_type>();
        }

        [[nodiscard]]
        constexpr const_pointer data() const noexcept
        {
            return cdata();
        }

        [[nodiscard]]
        constexpr const_pointer cdata() const noexcept
        {
            return buffer_.as_ptr<pixel_type>();
        }

        [[nodiscard]]
        constexpr view_type view() noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr const_pixline_type cline0() const noexcept
        {
            return { cdata(), space_type::line_size() };
        }

        [[nodiscard]]
        constexpr const_pixline_type line0() const noexcept
        {
            return cline0();
        }

        [[nodiscard]]
        constexpr pixline_type line0() noexcept
        {
            return { data(), space_type::line_size() };
        }

        [[nodiscard]]
        constexpr const_pixline_type cbegin() const noexcept
        {
            return cline0();
        }

        [[nodiscard]]
        constexpr const_pointer cend() const noexcept
        {
            return cdata() + space_type::size();
        }

        [[nodiscard]]
        constexpr const_pixline_type begin() const noexcept
        {
            return cbegin();
        }

        [[nodiscard]]
        constexpr const_pointer end() const noexcept
        {
            return cend();
        }

        [[nodiscard]]
        constexpr pixline_type begin() noexcept
        {
            return line0();
        }

        [[nodiscard]]
        constexpr const_pointer end() noexcept
        {
            return cend();
        }

        [[nodiscard]]
        constexpr const_view_type view() const noexcept
        {
            return cview();
        }

        [[nodiscard]]
        constexpr const_view_type cview() const noexcept
        {
            return *this;
        }


        template<class C>
        constexpr auto store(point2d position, const C& image) noexcept -> decltype(view().store(position, image))
        {
            return view().store(position, image);
        }

        template<class C>
        constexpr auto store(pxside_t x, pxside_t y, const C& image) noexcept -> decltype(view().store(x, y, image))
        {
            return view().store(x, y, image);
        }

        template<class C>
        constexpr auto store(const C& image) noexcept -> decltype(view().store(image))
        {
            return view().store(image);
        }

    private:
        [[nodiscard]]
        constexpr space_type& _space_ref() noexcept
        {
            return *this;
        }

        constexpr void _reject_space() noexcept
        {
            _space_ref() = {};
        }

    private:
        buffer_t buffer_;
    };

    using pix8map = pixmap<u8tint_t>;
}

using px::pixmap;
using px::pix8map;