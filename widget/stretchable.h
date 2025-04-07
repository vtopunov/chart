#pragma once

#include <widget/event.h>


namespace widget
{
    using stretchable_pxrectangle = ::rectangle<npx_t, pxoff_t>;

    template<class Pos, class Sz>
    [[nodiscard]] constexpr pxsizes stretchable_sizes(const rectangle<Pos, Sz>& r, pxsizes bound) noexcept
    {
        using overpxoff_t = int64_t;
        static_assert(std::is_signed_v<Sz>);
        static_assert(sizeof(overpxoff_t) > sizeof(Pos));
        static_assert(sizeof(overpxoff_t) > sizeof(Sz));
        static_assert(sizeof(overpxoff_t) > sizeof(npx_t));

        constexpr auto clamp_len = [] (overpxoff_t position, overpxoff_t fixlen, overpxoff_t len) noexcept
        {
            len -= position;
            if (len < 0LL) [[unlikely]]
                return 0_npx;

                if (fixlen <= 0LL)
                {
                    len += fixlen;

                    if (len < 0LL) [[unlikely]]
                        return 0_npx;
                }
                else
                {
                    if (fixlen < len)
                    {
                        len = fixlen;
                    }
                }

                return narrow<npx_t>(len);
        };

        return
        {
            clamp_len(r.x(), r.width(), bound.width()),
            clamp_len(r.y(), r.height(), bound.height())
        };
    }

    template<class Pos, class Sz, class... EArgs>
    [[nodiscard]] constexpr auto stretchable_sizes(const rectangle<Pos, Sz>& r, const basic_widget_event<EArgs...>& e) noexcept
        -> decltype(stretchable_sizes(r, e.content().sizes()))
    {
        return stretchable_sizes(r, e.content().sizes());
    }
}

using widget::stretchable_pxrectangle;
using widget::stretchable_sizes;
