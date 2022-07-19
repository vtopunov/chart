#pragma once

#include <px/fwd.h>
#include <px/alignment.h>

namespace px
{
    template<bool>
    class line_size_opt
    {
    public:
        template<size_t>
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

        template<size_t PxSize>
        [[nodiscard]] static constexpr line_size_opt instance_from_width(size_t width) noexcept
        {
            return { aligned_width<PxSize, default_alignment>(width) };
        }

        [[nodiscard]]
        constexpr size_t included_line_size() const noexcept
        {
            return line_size_;
        }

    private:
        size_t line_size_{};
    };

    template<size_t Test, size_t Base>
    using is_compatible_align_t = std::conjunction<std::negation<px::is_dynamic_alignment<Test>>, px::is_dynamic_alignment<Base>>;

    template<size_t Test, size_t Base>
    constexpr auto is_compatible_align_v = is_compatible_align_t<Test, Base>::value;

    template<size_t PxSize, size_t Alignment = default_alignment>
    class pixspace : private line_size_opt<px::is_dynamic_alignment_v<Alignment>>
    {
    public:
        static constexpr auto px_size = PxSize;
        static constexpr auto alignment = Alignment;
        using dynamic_alignment_is_enabled_t = px::is_dynamic_alignment<alignment>;
        static constexpr auto dynamic_alignment_is_enabled = dynamic_alignment_is_enabled_t::value;
        using line_size_type = line_size_opt<dynamic_alignment_is_enabled>;

        constexpr pixspace() noexcept = default;

        constexpr pixspace(const pixspace&) noexcept = default;

        template<size_t Align, std::enable_if_t <is_compatible_align_v<Align, alignment>, int > = 0 >
        constexpr pixspace(const pixspace<px_size, Align>& right) noexcept
            : line_size_type{ right._line_size_opt(dynamic_alignment_is_enabled_t{}) }
            , sizes_{ right.sizes() }
        {}

        constexpr pixspace(size2d sizes, line_size_type line_size) noexcept
            : line_size_type{ line_size }
            , sizes_{ sizes }
        {
            D_ASSERT(pixspace::width() <= pixspace::line_size());
        }

        constexpr pixspace(size2d sizes) noexcept
            : pixspace{ sizes, line_size_type::template instance_from_width<px_size>(sizes.width()) }
        {}

        constexpr pixspace(pxside_t w, pxside_t h) noexcept
            : pixspace{ size2d{ w, h } }
        {}


        constexpr pixspace(pxside_t w, pxside_t h, line_size_type line_size) noexcept
            : pixspace{ size2d{ w, h }, line_size }
        {}

        constexpr pixspace& operator = (const pixspace&) noexcept = default;

        template<size_t Align, std::enable_if_t<is_compatible_align_v<Align, alignment>, int> = 0>
        constexpr pixspace& operator = (const pixspace<px_size, Align>& right) noexcept
        {
            line_size_type::operator = (right._line_size_opt(dynamic_alignment_is_enabled_t{}));
            sizes_ = right.sizes();
            return *this;
        }

        [[nodiscard]]
        constexpr size_t line_size() const noexcept
        {
            if constexpr (dynamic_alignment_is_enabled)
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
        constexpr size2d sizes() const noexcept
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
        consteval line_size_opt<false> _line_size_opt(std::false_type) const
        {
            return {};   
        }

        constexpr line_size_opt<true> _line_size_opt(std::true_type) const
        {
            return line_size();
        }


    private:
        size2d sizes_{};
    };

    template<size_t PxSize, size_t Alignment>
    constexpr const pixspace<PxSize, Alignment>& space(const pixspace<PxSize, Alignment>& c) noexcept
    {
        return c;
    }

    using pix8space = pixspace<1_uz>;
    using pix32space = pixspace<4_uz>;
}

using px::pixspace;
using px::pix8space;
using px::pix32space;
