#pragma once

#include <core/buffer.h>

#include <px/pixspan.h>

namespace px
{
    template<class T, size_t Alignment = default_alignment>
    class pixmap : public pixspace<sizeof(T), Alignment>
    {
    public:
        using pixel_type = T;
        using const_pixel_type = std::add_const_t<T>;
        using space_type = pixspace<sizeof(T), Alignment>;
        using view_type = pixspan<T, Alignment>;
        using const_view_type = pixspan<const_pixel_type, Alignment>;
        using line_size_type = typename space_type::line_size_type;
        
        using pointer = pixel_type*;
        using const_pointer = const_pixel_type*;

        constexpr pixmap() noexcept = default;
        
        constexpr pixmap(const pixmap&) noexcept = delete;

        constexpr pixmap(pixmap&& right) noexcept 
            : space_type{ std::exchange(right._ref_space(), {}) }
            , buffer_{ std::move(right.buffer_) }
        {}

        explicit pixmap(const space_type& space) noexcept
            : space_type{ space }
            , buffer_{ buffer_construct, space.size_bytes() }
        {
            if (buffer_)
            {
                _zero_memory();
            }
            else
            {
                _reject_space();
            }
        }

        pixmap(byte_buffer_t buffer, const space_type& space) noexcept
            : space_type{ space }
            , buffer_{ std::move(buffer) }
        {
            if (try_reserve(buffer_, space.size_bytes()))
            {
                _zero_memory();
            }
            else
            {
                _reject_space();
            }
        }

        explicit pixmap(byte_buffer_t buffer) noexcept
            : pixmap{ std::move(buffer), space_type{} }
        {}

        explicit pixmap(size2d_t sizes) noexcept
            : pixmap{ space_type{ sizes } }
        {}

        pixmap(byte_buffer_t buffer, size2d_t sizes) noexcept
            : pixmap{ std::move(buffer), space_type{ sizes } }
        {}

        pixmap(pxside_t x, pxside_t y) noexcept
            : pixmap{ space_type{ x, y } }
        {}

        pixmap(byte_buffer_t buffer, pxside_t x, pxside_t y) noexcept
            : pixmap{ std::move(buffer), space_type{ x, y } }
        {}

        pixmap(size2d_t sizes, line_size_type line_size) noexcept
            : pixmap{ space_type{ sizes, line_size } }
        {}

        pixmap(byte_buffer_t buffer, size2d_t sizes, line_size_type line_size) noexcept
            : pixmap{ std::move(buffer), space_type{ sizes, line_size } }
        {}

        pixmap(pxside_t x, pxside_t y, line_size_type line_size) noexcept
            : pixmap{ space_type{ x, y, line_size } }
        {}

        pixmap(byte_buffer_t buffer, pxside_t x, pxside_t y, line_size_type line_size) noexcept
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
            std::swap(_ref_space(), right._ref_space());
            buffer_.swap(right.buffer_);
        }

        constexpr void swap(byte_buffer_t& right) noexcept
        {
            _reject_space();
            buffer_.swap(right);
        }

        [[nodiscard]]
        constexpr byte_buffer_t release_buffer() noexcept
        {
            _reject_space();
            return std::move(buffer_);
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
        constexpr view_type view() noexcept
        {
            return *this;
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


        template<class T>
        constexpr auto store(point2d_t position, const T& image) noexcept -> decltype(view().store(position, image))
        {
            return view().store(position, image);
        }

        template<class T>
        constexpr auto store(pxside_t x, pxside_t y, const T& image) noexcept -> decltype(view().store(x, y, image))
        {
            return view().store(x, y, image);
        }

        template<class T>
        constexpr auto store(const T& image) noexcept -> decltype(view().store(image))
        {
            return view().store(image);
        }

    private:
        void _zero_memory() noexcept
        {
            memset(buffer_.data(), 0, buffer_.size());
        }

        [[nodiscard]]
        constexpr space_type& _ref_space() noexcept
        {
            return *this;
        }

        constexpr void _reject_space() noexcept
        {
            _ref_space() = {};
        }

    private:
        byte_buffer_t buffer_;
    };

    using pix8map_t = pixmap<u8tint_t>;
}

using px::pixmap;
using px::pix8map_t;