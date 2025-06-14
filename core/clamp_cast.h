#pragma once

#include <core/utility.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast)
D_WARNING_DISABLE_MSVC(W_arithmetic_overflow);

template<class T0, class T1>
constexpr bool is_unsigned2_v = std::conjunction_v
<
    std::is_unsigned<T0>,
    std::is_unsigned<T1>
>;

template<class word, class dword>
[[nodiscard]] constexpr std::enable_if_t<
    is_unsigned2_v<word, dword>,
    word
> lo_cast(dword dw) noexcept
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


template<class word, class dword>
[[nodiscard]] constexpr std::enable_if_t<
    is_unsigned2_v<word, dword>,
    word
> hi_cast(dword dw) noexcept
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

template<class Source>
[[nodiscard]] constexpr auto clamp_to_unsigned(Source v) noexcept
{
    using source_t = std::remove_cvref_t<Source>;

    if constexpr (!std::is_unsigned_v<source_t>)
    {
        using unsigned_t = std::make_unsigned_t<source_t>;
        constexpr source_t zero{};
        constexpr unsigned_t u_zero{};

        if (v >= zero) [[likely]]
        {
            return static_cast<unsigned_t>(v);
        }
        else
        {
            return u_zero;
        }
    }
    else
    {
        return v;
    }
}

namespace private_detail_clamp_cast
{
    struct no_round_fn
    {
        template<class Source>
        [[nodiscard]] constexpr Source operator () (Source v) const noexcept
        {
            return v;
        }
    };

    template<class Target, class Source>
    [[nodiscard]] constexpr Target clamp_max_u2i_cast(Source v) noexcept
    {
        using source_t = std::remove_cvref_t<Source>;
        constexpr auto target_max = numeric_max_v<Target>;
        constexpr source_t target_max_source{ target_max };

        if (v <= target_max_source) [[likely]]
        {
            return static_cast<Target>(v);
        }
        else
        {
            return target_max;
        }
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target clamp_max_u2fp_cast(Source v) noexcept
    {
        using source_t = std::remove_cvref_t<Source>;
        using target_t = std::remove_cvref_t<Target>;
        constexpr source_t source_one{ 1u };
        constexpr auto target_max_source = (source_one << numeric_digits_v<target_t>) - source_one;
        constexpr auto target_max = static_cast<target_t>(target_max_source);

        if (v <= target_max_source) [[likely]]
        {
            return static_cast<Target>(v);
        }
        else
        {
            return target_max;
        }
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target clamp_minmax_i2fp_cast(Source v) noexcept
    {
        using source_t = std::remove_cvref_t<Source>;
        using target_t = std::remove_cvref_t<Target>;
        constexpr source_t source_one{ 1 };
        constexpr auto target_max_source = (source_one << numeric_digits_v<target_t>) - source_one;
        constexpr auto target_min_source = -target_max_source;
        constexpr auto target_max = static_cast<target_t>(target_max_source);
        constexpr auto target_min = static_cast<target_t>(target_min_source);

        if (v >= target_min_source) [[likely]]
        {
            if (v <= target_max_source) [[likely]]
            {
                return static_cast<Target>(v);
            }
            else
            {
                return target_max;
            }
        }
        else
        {
            return target_min;
        }
    }

    template<class Target, class Source, class RoundFn = no_round_fn>
    [[nodiscard]] constexpr Target clamp_minmax_fpi2i_cast(Source v, RoundFn round = {}) noexcept
    {
        using source_t = std::remove_cvref_t<Source>;
        using target_t = std::remove_cvref_t<Target>;
        constexpr auto source_is_floating_point = std::is_floating_point_v<source_t>;
        constexpr auto target_min = numeric_min_v<target_t>;
        constexpr auto target_max = numeric_max_v<target_t>;
        constexpr target_t target_zero{ 0 };
        constexpr target_t target_one{ 1 };
        constexpr int target_digits{ numeric_digits_v<target_t> };
        constexpr int source_digits{ numeric_digits_v<source_t> };
        constexpr int digits_loss{ (source_is_floating_point && (target_digits > source_digits)) ? (target_digits - source_digits) : 0 };
        constexpr auto round_mask = ~((target_one << digits_loss) - target_one);
        constexpr auto round_target_min = target_min & round_mask;
        constexpr auto round_target_max = target_max & round_mask;
        static_assert((digits_loss) ? (round_target_min >= target_min) : (round_target_min == target_min));
        static_assert((digits_loss) ? (round_target_max < target_max) : (round_target_max == target_max));

        constexpr auto target_min_source = static_cast<source_t>(round_target_min);
        constexpr auto target_max_source = static_cast<source_t>(round_target_max);
        static_assert(round_target_min == static_cast<target_t>(target_min_source));
        static_assert(round_target_max == static_cast<target_t>(target_max_source));

        if (v >= target_min_source) [[likely]]
        {
            if (v <= target_max_source) [[likely]]
            {
                if constexpr (source_is_floating_point)
                {
                    return static_cast<Target>(round(v));
                }
                else
                {
                    return static_cast<Target>(v);
                }
            }
            else
            {
                return round_target_max;
            }
        }
        else
        {
            if constexpr (source_is_floating_point && (round_target_min != target_zero))
            {
                const auto is_not_nan = (v < target_min_source);
                return (is_not_nan) ? round_target_min : target_zero;
            }
            else
            {
                return round_target_min;
            }
        }
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target clamp_u2u_cast(Source v) noexcept
    {
        static_assert(is_unsigned2_v<Target, Source>);

        if constexpr (sizeof(Source) > sizeof(Target))
        {
            return clamp_max_u2i_cast<Target, Source>(v);
        }
        else
        {
            return static_cast<Target>(v);
        }
    }

    template<class Target, class Source, class RoundFn = no_round_fn>
    [[nodiscard]] constexpr Target clamp_cast(Source v, RoundFn round = {}) noexcept
    {
        using namespace private_detail_clamp_cast;

        if constexpr (std::is_unsigned_v<Target>)
        {
            if constexpr (std::is_unsigned_v<Source>)
            {
                return clamp_u2u_cast<Target>(v);
            }
            else
            {
                if constexpr (std::is_integral_v<Source>)
                {
                    return clamp_u2u_cast<Target>(clamp_to_unsigned(v));
                }
                else
                {
                    static_assert(std::is_floating_point_v<Source>);
                    return clamp_minmax_fpi2i_cast<Target>(v, round);
                }
            }
        }
        else
        {
            if constexpr (std::is_integral_v<Target>)
            {
                if constexpr (std::is_unsigned_v<Source>)
                {
                    if constexpr (sizeof(Source) >= sizeof(Target))
                    {
                        return clamp_max_u2i_cast<Target>(v);
                    }
                    else
                    {
                        return v;
                    }
                }
                else
                {
                    if constexpr (std::is_integral_v<Source>)
                    {
                        if constexpr (sizeof(Source) > sizeof(Target))
                        {
                            return clamp_minmax_fpi2i_cast<Target>(v);
                        }
                        else
                        {
                            return v;
                        }
                    }
                    else
                    {
                        static_assert(std::is_floating_point_v<Source>);
                        return clamp_minmax_fpi2i_cast<Target>(v, round);
                    }
                }
            }
            else
            {
                if constexpr (std::is_floating_point_v<Target>)
                {
                    if constexpr (std::is_unsigned_v<Source>)
                    {
                        if constexpr (sizeof(Source) >= sizeof(Target))
                        {
                            return clamp_max_u2fp_cast<Target>(v);
                        }
                        else
                        {
                            return v;
                        }
                    }
                    else
                    {
                        if constexpr (std::is_integral_v<Source>)
                        {
                            if constexpr (sizeof(Source) >= sizeof(Target))
                            {
                                return clamp_minmax_i2fp_cast<Target>(v);
                            }
                            else
                            {
                                return v;
                            }
                        }
                        else
                        {
                            static_assert(std::is_floating_point_v<Source>);
                            static_assert(sizeof(Target) >= sizeof(Source));
                            return v;
                        }
                    }
                }
                else
                {
                    static_assert(std::is_same_v<std::remove_cvref_t<Target>, std::remove_cvref_t<Source>>);
                    return v;
                }
            }
        }
    }
}

using private_detail_clamp_cast::clamp_cast;

D_WARNING_POP
