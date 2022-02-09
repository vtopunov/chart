#pragma once

#include <core/ordered_overload.h>
#include <core/utility.h>

namespace private_detail_swap
{
    using namespace ordered_overload;

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, _order<_3>) noexcept -> decltype(as_reference((std::swap<R>(right, left), right)))
    {
        std::swap<R>(right, left);
        return right;
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, _order<_2>) noexcept -> decltype(as_reference((std::swap<L>(left, right), left)))
    {
        std::swap<L>(left, right);
        return left;
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, _order<_1>) noexcept -> decltype(as_reference((right.swap(left), right)))
    {
        right.swap(left);
        return right;
    }

    template<class L, class R>
    constexpr auto swap_impl(L& left, R& right, _order<_0>) noexcept -> decltype(as_reference((left.swap(right), left)))
    {
        left.swap(right);
        return left;
    }

    template<class L, class R>
    constexpr auto swap(L& left, R& right) noexcept -> decltype(swap_impl(left, right, _start))
    {
        return swap_impl(left, right, _start);
    }
}

template<class L, class R = L>
class temp_swap
{
public:
    D_DISABLE_COPY_MOVE(temp_swap);

    constexpr temp_swap(L& left, R& right) noexcept
        : left_{ left }
        , right_{ right }
    {
        private_detail_swap::swap(left_, right_);
    }

    constexpr ~temp_swap() noexcept
    {
        private_detail_swap::swap(right_, left_);
    }

    [[nodiscard]]
    constexpr L& left() const noexcept
    {
        return left_;
    }

    [[nodiscard]]
    constexpr R& right() const noexcept
    {
        return right_;
    }

private:
    L& left_;
    R& right_;
};

template<class L>
temp_swap(L&, L&)->temp_swap<L>;

template<class L, class R>
temp_swap(L&, R&)->temp_swap<L, R>;