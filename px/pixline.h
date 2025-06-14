#pragma once

#include <px/fwd.h>


namespace px
{
    template<class T>
    class pixline
    {
    public:
        using span_type = span<T>;
        using const_span_type = span<const T>;
        using pixpointer = T*;
        using const_pixpointer = const T*;

        D_DEFAULT_ALL_CAEQ(pixline);

        constexpr pixline(pixpointer data, size_t size, size_t width) noexcept
            : data_{ data }
            , size_{ size }
            , width_{ width }
        {
            D_ASSERT_OR_ASSUME(width_ <= size_);
        }

        [[nodiscard]]
        constexpr pixpointer data() const noexcept
        {
            return data_;
        }

        [[nodiscard]]
        constexpr size_t size() const noexcept
        {
            return size_;
        }

        [[nodiscard]]
        constexpr size_t width() const noexcept
        {
            return width_;
        }

        [[nodiscard]]
        constexpr span_type pixels() const noexcept
        {
            return { data_, width_ };
        }

        [[nodiscard]]
        constexpr const_span_type cpixels() const noexcept
        {
            return { data_, width_ };
        }

        [[nodiscard]]
        constexpr bool operator==(const_pixpointer right) const noexcept
        {
            return data() == right;
        }

        [[nodiscard]]
        constexpr bool operator!=(const_pixpointer right) const noexcept
        {
            return data() != right;
        }

        [[nodiscard]]
        constexpr pixline with_position(pixpointer position) const noexcept
        {
            return { position, size(), width() };
        }

        template<class Diff>
        [[nodiscard]] std::enable_if_t<std::is_integral_v<Diff>, pixline> forward(Diff value) const noexcept
        {
            return with_position(data() + value);
        }

        template<class Diff>
        [[nodiscard]] std::enable_if_t<std::is_integral_v<Diff>, pixline> backward(Diff value) const noexcept
        {
            return with_position(data() - value);
        }

        [[nodiscard]]
        constexpr pixline next(ptrdiff_t index) const noexcept
        {
            return forward(index * narrow<ptrdiff_t>(size()));
        }

        [[nodiscard]]
        constexpr pixline next() const noexcept
        {
            return forward(size());
        }

        [[nodiscard]]
        constexpr pixline prev(ptrdiff_t index) const noexcept
        {
            return backward(index * narrow<ptrdiff_t>(size()));
        }

        [[nodiscard]]
        constexpr pixline prev() const noexcept
        {
            return backward(size());
        }

        constexpr pixline& operator++() noexcept
        {
            return operator=(next());
        }

        constexpr pixline& operator--() noexcept
        {
            return operator=(prev());
        }

        [[nodiscard]]
        constexpr pixline operator++(int) noexcept
        {
            auto temp = *this;
            operator++();
            return temp;
        }

        [[nodiscard]]
        constexpr pixline operator--(int) noexcept
        {
            auto temp = *this;
            operator--();
            return temp;
        }

        constexpr pixline& operator+=(ptrdiff_t index) noexcept
        {
            return operator=(next(index));
        }

        constexpr pixline& operator-=(ptrdiff_t index) noexcept
        {
            return operator=(prev(index));
        }

        [[nodiscard]]
        constexpr span_type operator*() const noexcept
        {
            return pixels();
        }

    private:
        T* data_{ nullptr };
        size_t size_{};
        size_t width_{};
    };

    template<class T>
    [[nodiscard]] constexpr bool operator==(const T* left, pixline<T> right) noexcept
    {
        return right == left;
    }

    template<class T>
    [[nodiscard]] constexpr bool operator!=(const T* left, pixline<T> right) noexcept
    {
        return right != left;
    }

    template<class T>
    [[nodiscard]] constexpr pixline<T> operator + (pixline<T> right, ptrdiff_t left) noexcept
    {
        return right.next(left);
    }

    template<class T>
    [[nodiscard]] constexpr pixline<T> operator + (ptrdiff_t right, pixline<T> left) noexcept
    {
        return left + right;
    }

    template<class T>
    [[nodiscard]] constexpr pixline<T> operator - (pixline<T> right, ptrdiff_t left) noexcept
    {
        return right.prev(left);
    }

    using pix8line = pixline<luminance_t>;
    using const_pix8line = pixline<const luminance_t>;
}

using px::pixline;
using px::pix8line;
using px::const_pix8line;