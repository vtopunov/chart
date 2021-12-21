#pragma once

#include <px/pxfwd.h>
#include <px/pxalignment.h>

namespace px
{
    template<bool>
    class line_size_opt
    {
    public:
        template<size_t, size_t>
        [[nodiscard]] static constexpr line_size_opt instance_from_width(size_t) noexcept
        {
            return {};
        }
    };

    template<>
    class line_size_opt<true>
    {
    public:
        constexpr line_size_opt() noexcept = default;

        constexpr line_size_opt(size_t line_size) noexcept
            : line_size_{ line_size }
        {}

        template<size_t PxSize, size_t Alignment>
        [[nodiscard]] static constexpr line_size_opt instance_from_width(size_t width) noexcept
        {
            return { aligned_width<PxSize, Alignment>(width) };
        }

        [[nodiscard]]
        constexpr size_t included_line_size() const noexcept
        {
            return line_size_;
        }

    private:
        size_t line_size_{};
    };


    template<size_t PxSize, size_t Alignment = default_alignment>
    class pixspace : private line_size_opt<is_dynamic_alignment_v<Alignment>>
    {
    public:
        static constexpr auto px_size = PxSize;
        static constexpr auto alignment = Alignment;
        static constexpr auto is_dynamic_alignment = is_dynamic_alignment_v<alignment>;
        static constexpr auto static_or_default_alignment = is_dynamic_alignment ? default_alignment : alignment;
        using line_size_type = line_size_opt<is_dynamic_alignment>;

        constexpr pixspace() noexcept = default;

        constexpr pixspace(const pixspace&) noexcept = default;

        template<size_t Align, class = std::enable_if_t<(is_dynamic_alignment) && !is_dynamic_alignment_v<Align>>>
        constexpr pixspace(const pixspace<px_size, Align>& right) noexcept
            : line_size_type{ right.line_size() }
            , sizes_{ right.sizes() }
        {}

        constexpr pixspace(size2d_t sizes, line_size_type line_size) noexcept
            : line_size_type{ line_size }
            , sizes_{ sizes }
        {
            D_ASSERT(pixspace::width() <= pixspace::line_size());
        }

        constexpr pixspace(size2d_t sizes) noexcept
            : pixspace{ sizes, line_size_type::instance_from_width<px_size, static_or_default_alignment>(sizes.width()) }
        {}

        constexpr pixspace(pxside_t w, pxside_t h) noexcept
            : pixspace{ size2d_t{ w, h } }
        {}


        constexpr pixspace(pxside_t w, pxside_t h, line_size_type line_size) noexcept
            : pixspace{ size2d_t{ w, h }, line_size }
        {}

        constexpr pixspace& operator = (const pixspace&) noexcept = default;

        template<size_t Align, class = std::enable_if_t<(is_dynamic_alignment) && !is_dynamic_alignment_v<Align>>>
        constexpr pixspace& operator = (const pixspace<px_size, Align>& right) noexcept
        {
            line_size_type::operator = (line_size_type{ right.line_size() });
            sizes_ = right.sizes();
            return *this;
        }

        [[nodiscard]]
        constexpr size_t line_size() const noexcept
        {
            if constexpr (is_dynamic_alignment)
            {
                return line_size_type::included_line_size();
            }
            else
            {
                return aligned_width<px_size, alignment>(this->width());
            }
        }

        [[nodiscard]]
        constexpr size_t size() const noexcept
        {
            return line_size() * height();
        }

        [[nodiscard]]
        constexpr size_t size_bytes() const noexcept
        {
            return size_mul<px_size>(size());
        }

        [[nodiscard]]
        constexpr size2d_t sizes() const noexcept
        {
            return sizes_;
        }

        [[nodiscard]]
        constexpr pxside_t width() const noexcept
        {
            return sizes_.width();
        }

        [[nodiscard]]
        constexpr pxside_t height() const noexcept
        {
            return sizes_.height();
        }

    private:
        size2d_t sizes_{};
    };

    template<size_t PxSize, size_t Alignment>
    constexpr const pixspace<PxSize, Alignment>& space(const pixspace<PxSize, Alignment>& c) noexcept
    {
        return c;
    }

    using pix8space_t = pixspace<1_uz>;
    using pix32space_t = pixspace<4_uz>;
}

using px::pixspace;
using px::pix8space_t;
using px::pix32space_t;
