#pragma once

#include <core/warnings.h>
#include <core/limits.h>
#include <core/zero.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast)

template<class Target, class Source>
constexpr bool is_unsigned2_v = std::is_unsigned_v<Source> && std::is_unsigned_v<Target>;

template<class word, class dword> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<word, dword>, word> hi_cast(dword dw) noexcept
{
    if constexpr (sizeof(dword) > sizeof(word))
    {
        constexpr auto nbits = 8u * (sizeof(dword) - sizeof(word));

        return static_cast<word>(dw >> nbits);
    }
    else
    {
        return zero_v<word>;
    }
}

template<class word, class dword> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<word, dword>, word> lo_cast(dword dw) noexcept
{
    if constexpr (sizeof(dword) > sizeof(word))
    {
        constexpr auto mask = static_cast<dword>(word(-1));

        return static_cast<word>(dw & mask);
    }
    else
    {
        return static_cast<word>(dw);
    }
}

template<class Target, class Source> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<Target, Source>, Target> clamp_cast(Source v) noexcept
{
    if constexpr (sizeof(Source) > sizeof(Target))
    {
        constexpr Source target_max{ numeric_max_v<Target> };

        return (v > target_max) ? target_max : static_cast<Target>(v);
    }
    else
    {
        return static_cast<Target>(v);
    }
}

D_WARNING_POP
