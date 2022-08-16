#pragma once

#include <core/type_traits.h>
#include <core/warnings.h>
#include <core/limits.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast)

template<class T0, class T1>
constexpr bool is_unsigned2_v = std::conjunction_v
<
    std::is_unsigned<T0>,
    std::is_unsigned<T1>
>;

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


template<class word, class dword> [[nodiscard]]
constexpr std::enable_if_t<is_unsigned2_v<word, dword>, word> hi_cast(dword dw) noexcept
{
    if constexpr (sizeof(dword) > sizeof(word))
    {
        constexpr auto nbits = 8u * sizeof(word);
        return lo_cast<word>(dw >> nbits);
    }
    else
    {
        return {};
    }
}

template<class Source> [[nodiscard]]
constexpr decltype(auto) clamp_to_unsigned(Source v) noexcept
{
    using source_t = std::remove_cvref_t<Source>;

    if constexpr (!std::is_unsigned_v<source_t>)
    {
        using unsigned_t = std::make_unsigned_t<source_t>;
        constexpr source_t zero{};
        return static_cast<unsigned_t>((v < zero) ? zero : v);
    }
    else
    {
        return v;
    }
}

namespace private_detail_clamp_cast
{
    template<class Target, class Source>
    [[nodiscard]] constexpr Target clamp_max_cast(Source v) noexcept
    {
        using source_t = std::remove_cvref_t<Source>;
        constexpr auto target_max = numeric_max_v<Target>;
        constexpr source_t target_max_source{ target_max };
        return (target_max_source < v) ?  target_max : static_cast<Target>(v);
    }


    template<class Target, class Source>
    [[nodiscard]] constexpr Target clamp_minmax_cast(Source v) noexcept
    {
        using source_t = std::remove_cvref_t<Source>;
        constexpr auto target_min = numeric_min_v<Target>;
        constexpr auto target_max = numeric_max_v<Target>;
        constexpr source_t target_min_source{ target_min };
        constexpr source_t target_max_source{ target_max };
        return (v < target_min_source) ? target_min : ((target_max_source < v) ? target_max : static_cast<Target>(v));
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target clamp_cast_uu(Source v) noexcept
    {
        static_assert(is_unsigned2_v<Target, Source>);

        if constexpr (sizeof(Source) > sizeof(Target))
        {
            return clamp_max_cast<Target, Source>(v);
        }
        else
        {
            return static_cast<Target>(v);
        }
    }
}

template<class Target, class Source> [[nodiscard]]
constexpr Target clamp_cast(Source v) noexcept
{
    using namespace private_detail_clamp_cast;

    if constexpr (std::is_unsigned_v<Target>)
    {
        if constexpr (std::is_unsigned_v<Source>)
        {
            return clamp_cast_uu<Target>(v);
        }
        else
        {
            if constexpr (std::is_integral_v<Source>)
            {
                return clamp_cast_uu<Target>(clamp_to_unsigned(v));
            }
            else
            {
                static_assert(std::is_floating_point_v<Source>);
                return clamp_minmax_cast<Target>(v);
            }
        }
    }
    else
    {
        static_assert(std::is_integral_v<Target>);

        if constexpr (std::is_unsigned_v<Source>)
        {
            if constexpr (sizeof(Source) >= sizeof(Target))
            {
                return clamp_max_cast<Target>(v);
            }
            else
            {
                return static_cast<Target>(v);
            }
        }
        else
        {
            if constexpr (std::is_integral_v<Source>)
            {
                if constexpr (sizeof(Source) > sizeof(Target))
                {
                    return clamp_minmax_cast<Target>(v);
                }
                else
                {
                    return v;
                }
            }
            else
            {
                static_assert(std::is_floating_point_v<Source>);
                return clamp_minmax_cast<Target>(v);
            }
        }
    }
}


D_WARNING_POP
