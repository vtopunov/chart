#pragma once

#include <core/type_traits.h>
#include <core/warnings.h>
#include <core/limits.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast)

template<class Target, class Source>
constexpr bool is_unsigned2_v = std::conjunction_v
<
    std::is_unsigned<Source>,
    std::is_unsigned<Target>
>;

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
        return {};
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

        return static_cast<Target>((target_max < v) ? target_max : v);
    }
    else
    {
        return static_cast<Target>(v);
    }
}

template<class Source> [[nodiscard]]
constexpr decltype(auto) clamp_to_unsigned(Source v) noexcept
{
    using source_t = std::remove_cv_t<Source>;
    using unsigned_t = add_unsigned_t<source_t>;

    if constexpr (!std::is_same_v<unsigned_t, source_t>)
    {
        constexpr Source zero{};
        return static_cast<unsigned_t>((v < zero) ? zero : v);
    }
    else
    {
        return static_cast<unsigned_t>(v);
    }
}

D_WARNING_POP
