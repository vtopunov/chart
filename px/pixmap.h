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
        using view_type = const_span_type;
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
            if (buffer_) [[likely]]
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
            if (buffer_.try_reserve(space.size_bytes())) [[likely]]
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

        explicit pixmap(pxsize2d sizes) noexcept
            : pixmap{ space_type{ sizes } }
        {}

        pixmap(buffer_t buffer, pxsize2d sizes) noexcept
            : pixmap{ std::move(buffer), space_type{ sizes } }
        {}

        pixmap(pxsize_t x, pxsize_t y) noexcept
            : pixmap{ space_type{ x, y } }
        {}

        pixmap(buffer_t buffer, pxsize_t x, pxsize_t y) noexcept
            : pixmap{ std::move(buffer), space_type{ x, y } }
        {}

        pixmap(pxsize2d sizes, line_size_type line_size) noexcept
            : pixmap{ space_type{ sizes, line_size } }
        {}

        pixmap(buffer_t buffer, pxsize2d sizes, line_size_type line_size) noexcept
            : pixmap{ std::move(buffer), space_type{ sizes, line_size } }
        {}

        pixmap(pxsize_t x, pxsize_t y, line_size_type line_size) noexcept
            : pixmap{ space_type{ x, y, line_size } }
        {}

        pixmap(buffer_t buffer, pxsize_t x, pxsize_t y, line_size_type line_size) noexcept
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
            return data() + space_type::size();
        }

        template<class C>
        constexpr auto store(pxpoint2d position, const C& image) noexcept -> decltype(std::declval<span_type>().store(position, image))
        {
            return _span().store(position, image);
        }

        template<class C>
        constexpr auto store(pxsize_t x, pxsize_t y, const C& image) noexcept -> decltype(std::declval<span_type>().store(x, y, image))
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

    private:
        buffer_t buffer_;
    };

    using pix8map = pixmap<pix8_t>;

    static_assert(std::is_same_v<view_t<pix8map>, const pix8map::view_type>);
}

using px::pixmap;
using px::pix8map;