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
        using line_type = typename span_type::line_type;
        using const_line_type = typename const_span_type::line_type;
        using line_span_type = typename line_type::span_type;
        using const_line_span_type = typename const_line_type::const_span_type;
        using view_type = const_span_type;
        using line_size_type = typename space_type::line_size_type;

        using pointer = pixel_type*;
        using const_pointer = const_pixel_type*;

        constexpr pixmap() noexcept = default;

        constexpr pixmap(const pixmap&) noexcept = delete;

        constexpr pixmap(pixmap_construct_t, byte_buffer&& buffer, const space_type& space) noexcept
            : space_type{ space }
            , buffer_{ std::move(buffer) }
        {
            D_ASSERT(space.size_bytes() <= buffer_.size());
        }

        constexpr pixmap(pixmap&& right) noexcept
            : space_type{ std::exchange(right._space_ref(), {}) }
            , buffer_{ std::move(right.buffer_) }
        {}

        constexpr explicit pixmap(const space_type& space) noexcept
            : space_type{ space }
            , buffer_{ space.size_bytes() }
        {
            if (buffer_) [[likely]]
            {
                zero_memory(*this);
            }
            else
            {
                _reject_space();
            }
        }

        constexpr pixmap(byte_buffer&& buffer, const space_type& space) noexcept
            : space_type{ space }
            , buffer_{ std::move(buffer) }
        {
            if (buffer_.try_reserve(space.size_bytes())) [[likely]]
            {
                zero_memory(*this);
            }
            else
            {
                _reject_space();
            }
        }

        constexpr explicit pixmap(byte_buffer&& buffer) noexcept
            : pixmap{ std::move(buffer), space_type{} }
        {}

        constexpr explicit pixmap(pxsize2d sizes) noexcept
            : pixmap{ space_type{ sizes } }
        {}

        constexpr pixmap(byte_buffer&& buffer, pxsize2d sizes) noexcept
            : pixmap{ std::move(buffer), space_type{ sizes } }
        {}

        constexpr pixmap(npx_t x, npx_t y) noexcept
            : pixmap{ space_type{ x, y } }
        {}

        constexpr pixmap(byte_buffer&& buffer, npx_t x, npx_t y) noexcept
            : pixmap{ std::move(buffer), space_type{ x, y } }
        {}

        constexpr pixmap(pxsize2d sizes, line_size_type line_size) noexcept
            : pixmap{ space_type{ sizes, line_size } }
        {}

        constexpr pixmap(byte_buffer&& buffer, pxsize2d sizes, line_size_type line_size) noexcept
            : pixmap{ std::move(buffer), space_type{ sizes, line_size } }
        {}

        constexpr pixmap(npx_t x, npx_t y, line_size_type line_size) noexcept
            : pixmap{ space_type{ x, y, line_size } }
        {}

        constexpr pixmap(byte_buffer&& buffer, npx_t x, npx_t y, line_size_type line_size) noexcept
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

        constexpr void swap(byte_buffer& right) noexcept
        {
            _reject_space();
            buffer_.swap(right);
        }

        [[nodiscard]]
        byte_buffer release_buffer() noexcept
        {
            _reject_space();
            return std::exchange(buffer_, nullmem);
        }

        [[nodiscard]]
        constexpr pointer data() noexcept
        {
            return static_cast<pointer>(buffer_.void_data());
        }

        [[nodiscard]]
        constexpr const_pointer data() const noexcept
        {
            return cdata();
        }

        [[nodiscard]]
        constexpr const_pointer cdata() const noexcept
        {
            return static_cast<const_pointer>(buffer_.cvoid_data());
        }

        [[nodiscard]]
        constexpr const_line_type clines() const noexcept
        {
            return _cspan().lines();
        }

        [[nodiscard]]
        constexpr const_line_type lines() const noexcept
        {
            return clines();
        }

        [[nodiscard]]
        constexpr line_type lines() noexcept
        {
            return _span().lines();
        }

        [[nodiscard]]
        constexpr const_line_type clines(npx_t index) const noexcept
        {
            return _cspan().lines(index);
        }

        [[nodiscard]]
        constexpr const_line_type lines(npx_t index) const noexcept
        {
            return clines(index);
        }

        [[nodiscard]]
        constexpr line_type lines(npx_t index) noexcept
        {
            return _span().lines(index);
        }

        [[nodiscard]]
        constexpr const_line_span_type cline() const noexcept
        {
            return clines().cpixels();
        }

        [[nodiscard]]
        constexpr const_line_span_type line() const noexcept
        {
            return cline();
        }

        [[nodiscard]]
        constexpr line_span_type line() noexcept
        {
            return lines().pixels();
        }

        [[nodiscard]]
        constexpr const_line_span_type cline(npx_t index) const noexcept
        {
            return clines(index).cpixels();
        }

        [[nodiscard]]
        constexpr const_line_span_type line(npx_t index) const noexcept
        {
            return cline(index);
        }

        [[nodiscard]]
        constexpr line_span_type line(npx_t index) noexcept
        {
            return lines(index).pixels();
        }

        [[nodiscard]]
        constexpr const_line_type cbegin() const noexcept
        {
            return clines();
        }

        [[nodiscard]]
        constexpr const_pointer cend() const noexcept
        {
            return _cspan().end();
        }

        [[nodiscard]]
        constexpr const_line_type begin() const noexcept
        {
            return cbegin();
        }

        [[nodiscard]]
        constexpr const_pointer end() const noexcept
        {
            return cend();
        }

        [[nodiscard]]
        constexpr line_type begin() noexcept
        {
            return lines();
        }

        [[nodiscard]]
        constexpr const_pointer end() noexcept
        {
            return _span().end();
        }

        template<class C>
        constexpr auto store(pxpoint2d position, const C& image) noexcept -> decltype(std::declval<span_type>().store(position, image))
        {
            return _span().store(position, image);
        }

        template<class C>
        constexpr auto store(npx_t x, npx_t y, const C& image) noexcept -> decltype(std::declval<span_type>().store(x, y, image))
        {
            return _span().store(x, y, image);
        }

        template<class C>
        constexpr auto store(const C& image) noexcept -> decltype(std::declval<span_type>().store(image))
        {
            return _span().store(image);
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

        [[nodiscard]]
        constexpr span_type _span() noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr const_span_type _cspan() const noexcept
        {
            return *this;
        }

    private:
        byte_buffer buffer_;
    };

    using lumpixmap = pixmap<luminance_t>;
    using rgba_color_pixmap = pixmap<rgba_color>;

    static_assert(std::is_same_v<decl_view_type_t<lumpixmap>, lumpixmap::view_type>);
}

using px::pixmap;
using px::lumpixmap;
using px::rgba_color_pixmap;