#pragma once

#include <widget/event.h>


namespace widget
{
    using stretchable_pxrectangle = ::rectangle<pxside_t, pxoff_t>;

    template<class Pos, class Sz>
    constexpr pxsize2d stretchable_sizes(const rectangle<Pos, Sz>& r, pxsize2d bound) noexcept
    {
        using overpxoff_t = int64_t;
        static_assert(std::is_signed_v<Sz>);
        static_assert(sizeof(overpxoff_t) > sizeof(Pos));
        static_assert(sizeof(overpxoff_t) > sizeof(Sz));
        static_assert(sizeof(overpxoff_t) > sizeof(pxside_t));

        constexpr auto clamp_len = [] (overpxoff_t position, overpxoff_t fixlen, overpxoff_t len) noexcept
        {
            len -= position;
            if (len < 0LL) [[unlikely]]
                return 0_px;

                if (fixlen <= 0LL)
                {
                    len += fixlen;

                    if (len < 0LL) [[unlikely]]
                        return 0_px;
                }
                else
                {
                    if (fixlen < len)
                    {
                        len = fixlen;
                    }
                }

                return narrow<pxside_t>(len);
        };

        return
        {
            clamp_len(r.x(), r.width(), bound.width()),
            clamp_len(r.y(), r.height(), bound.height())
        };
    }

    template<class Pos, class Sz, class EventBase, class... EventExts>
    constexpr auto stretchable_sizes(const rectangle<Pos, Sz>& r, const widget_event<EventBase, EventExts...>& e) noexcept
        -> decltype(stretchable_sizes(r, e.template get<content_size2d>()))
    {
        return stretchable_sizes(r, e.template get<content_size2d>());
    }
}

using widget::stretchable_pxrectangle;
using widget::stretchable_sizes;
