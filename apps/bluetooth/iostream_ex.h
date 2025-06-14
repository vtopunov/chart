#pragma once

#include <string_view>
#include <string>
#include <iostream>

#include <core/type_traits.h>


template<class OutChar, class InChar>
std::enable_if_t<
    is_less_v<tr_sizeof<InChar>, tr_sizeof<OutChar>>,
    std::basic_ostream<OutChar>&
> operator << (std::basic_ostream<OutChar>& out, std::basic_string_view<InChar> in) noexcept
{
    for (const auto& in_c : in)
    {
        out << static_cast<OutChar>(in_c);
    }

    return out;
}

template<class OutChar, class InChar>
auto operator << (std::basic_ostream<OutChar>& out, const std::basic_string<InChar>& in) noexcept
-> decltype(out << std::basic_string_view<InChar>(in))
{
    return out << std::basic_string_view<InChar>(in);
}
