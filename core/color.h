#pragma once

#include <core/rational.h>


template<class T>
[[nodiscard]] constexpr T tint_max() noexcept
{
    if constexpr (std::is_floating_point_v<T>)
    {
        return static_cast<T>(1.0);
    }
    else
    {
        constexpr auto byte_max = numeric_max_v<u8tint_t>;

        if constexpr (is_rational_v<T>)
        {
            return rational_cast<T>(byte_max);
        }
        else
        {
            static_assert(std::is_integral_v<T>);
            return narrow<T>(byte_max);
        }
    }
}

template<class T = void>
struct tint_max_t
{
    constexpr operator T () const noexcept
    {
        return tint_max<T>();
    }
};

template<>
struct tint_max_t<void>
{
    template<class T>
    constexpr operator T () const noexcept
    {
        return tint_max<T>();
    }
};

template<class T = void>
constexpr tint_max_t<T> tint_max_v{};


template<class Target, class Source>
[[nodiscard]] constexpr Target color_cast(const Source& src) noexcept;

template<class T>
struct rgba_color
{
    static constexpr size_t extent{ 4u };

    using tint_type = T;
    using view_type = span<const tint_type, extent>;

    tint_type r;
    tint_type g;
    tint_type b;
    tint_type a;

    [[nodiscard]]
    constexpr bool operator == (const rgba_color&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const rgba_color&) const noexcept = default;

    template<class U, std::enable_if_t<is_safe_numeric_not_same_conversion_v<U, tint_type>, int> = 0>
    [[nodiscard]] constexpr operator rgba_color<U>() const noexcept
    {
        return color_cast<rgba_color<U>>(*this);
    }

    template<class U, std::enable_if_t<is_safe_numeric_not_same_conversion_v<U, tint_type>, int> = 0>
    [[nodiscard]] constexpr operator rgba_color<rational<U>>() const noexcept
    {
        return color_cast<rgba_color<rational<U>>>(*this);
    }

    [[nodiscard]]
    constexpr operator view_type() const noexcept
    {
        static_assert(sizeof(rgba_color<T>) == extent * sizeof(tint_type));
        return view_type{ std::addressof(r), extent };
    }

    [[nodiscard]]
    constexpr rgba_color with_red(tint_type value) const noexcept
    {
        return { value, g, b, a };
    }

    [[nodiscard]]
    constexpr rgba_color with_green(tint_type value) const noexcept
    {
        return { r, value, b, a };
    }

    [[nodiscard]]
    constexpr rgba_color with_blue(tint_type value) const noexcept
    {
        return { r, g, value, a };
    }

    [[nodiscard]]
    constexpr rgba_color with_alpha(tint_type value) const noexcept
    {
        return { r, g, b, value };
    }
};

using rgba_color32_view = rgba_color32_t::view_type;

static_assert(sizeof(rgba_color32_t) == 4u);


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast)

[[nodiscard]]
constexpr u8tint_t a_argb32(u32argb_t argb) noexcept
{
    return static_cast<u8tint_t>(argb >> 24);
}

[[nodiscard]]
constexpr u8tint_t r_argb32(u32argb_t argb) noexcept
{
    return static_cast<u8tint_t>((argb >> 16) & 0xffu);
}

[[nodiscard]]
constexpr u8tint_t g_argb32(u32argb_t argb) noexcept
{
    return static_cast<u8tint_t>((argb >> 8) & 0xffu);
}

[[nodiscard]]
constexpr u8tint_t b_argb32(u32argb_t argb) noexcept
{
    return static_cast<u8tint_t>(argb & 0xffu);
}

D_WARNING_POP


[[nodiscard]]
constexpr rgba_color32_t u32argb_to_color(u32argb_t argb) noexcept
{
    return
    {
        .r{ r_argb32(argb) },
        .g{ g_argb32(argb) },
        .b{ b_argb32(argb) },
        .a{ a_argb32(argb) }
    };
}

[[nodiscard]]
constexpr rgba_color32_t u32rgb_to_color(u32argb_t rgb) noexcept
{
    D_ASSERT(!a_argb32(rgb));
    return
    {
        .r{ r_argb32(rgb) },
        .g{ g_argb32(rgb) },
        .b{ b_argb32(rgb) },
        .a{ tint_max_v<> }
    };
}

[[nodiscard]]
constexpr u32argb_t color_to_u32argb(rgba_color32_t color) noexcept
{
    return static_cast<u32argb_t>(color.a) << 24
        | static_cast<u32argb_t>(color.r) << 16
        | static_cast<u32argb_t>(color.g) << 8
        | static_cast<u32argb_t>(color.b);
}


template <class T>
struct is_rgba_color : std::false_type
{};

template <class T>
struct is_rgba_color<rgba_color<T>> : std::true_type
{};

template <class T>
struct is_rgba_color<const T> : is_rgba_color<T>
{};

template<class T>
constexpr bool is_rgba_color_v = is_rgba_color<T>::value;

template<class Target, class Source>
[[nodiscard]] constexpr Target color_tint_cast(Source source) noexcept
{
    if constexpr (std::is_floating_point_v<Target>)
    {
        constexpr auto target_max = tint_max<Target>();

        if constexpr (is_rational_v<Source>)
        {
            using rational_int_type_t = typename Source::int_type;
            constexpr auto source_max = tint_max<rational_int_type_t>();
            return target_max * rational_cast<Target>(source / source_max);
        }
        else
        {
            if constexpr (std::is_integral_v<Source>)
            {
                constexpr auto source_max = tint_max<Source>();
                constexpr auto ratio = target_max / narrow<Target>(source_max);
                return ratio * narrow<Target>(source);
            }
            else
            {
                return narrow<Target>(source);
            }
        }
    }
    else
    {
        constexpr auto target_is_integral = std::is_integral_v<Target>;
        constexpr auto source_is_integral = std::is_integral_v<Source>;

        if constexpr (target_is_integral && source_is_integral)
        {
            return narrow<Target>(source);
        }
        else
        {
            static_assert(target_is_integral || is_rational_v<Target>);
            static_assert(source_is_integral || is_rational_v<Source>);
            return rational_cast<Target>(source);
        }
    }
}


template<class Target, class Source>
[[nodiscard]] constexpr Target color_cast(const Source& src) noexcept
{
    if constexpr (is_rgba_color_v<Target>)
    {
        using target_tint_t = typename Target::tint_type;

        if constexpr (is_rgba_color_v<Source>)
        {
            using source_tint_t = typename Source::tint_type;

            constexpr auto tint_cast = [] (source_tint_t tint) noexcept
            {
                return color_tint_cast<target_tint_t>(tint);
            };

            return
            {
                .r{ tint_cast(src.r) },
                .g{ tint_cast(src.g) },
                .b{ tint_cast(src.b) },
                .a{ tint_cast(src.a) }
            };
        }
        else
        {
            static_assert(std::is_integral_v<Source>);
            const auto argb_uint32 = narrow<u32argb_t>(src);
            const auto rgba_color32 = u32argb_to_color(argb_uint32);
            return color_cast<Target>(rgba_color32);
        }
    }
    else
    {
        static_assert(std::is_integral_v<Target> && is_rgba_color_v<Source>);
        const auto rgba_color32 = color_cast<rgba_color32_t>(src);
        const auto argb_uint32 = color_to_u32argb(rgba_color32);
        return narrow<Target>(argb_uint32);
    }
}

template<class T>
[[nodiscard]] constexpr rgba_colorf_t to_colorf(const rgba_color<T>& source) noexcept
{
    return color_cast<rgba_colorf_t>(source);
}

template<class T>
[[nodiscard]] constexpr rgba_color<T> inverse(const rgba_color<T>& c) noexcept
{
    constexpr auto tint_max_c = tint_max<T>();
    D_ASSERT(tint_max_c == c.a);

    return
    {
        .r{ tint_max_c - c.r },
        .g{ tint_max_c - c.g },
        .b{ tint_max_c - c.b },
        .a{ tint_max_c }
    };
}

namespace private_detail_argb_color
{
    template<class T>
    using signed_int0_t = std::conditional_t<
        is_narrowing_or_same_v<T, ptrdiff_t>, ptrdiff_t,
        std::conditional_t<is_narrowing_or_same_v<T, int64_t>, int64_t, intmax_t>
    >;

    template<class T>
    using signed_t = conditional_op_t<std::is_integral_v<T>, signed_int0_t, T>;

    template<class L, class R>
    using common_signed_t = std::common_type_t<signed_t<L>, signed_t<R>>;

    template<class L, class R>
    struct common_rational_deductor
    {
        using RDL = rational_detector<L>;
        using RDR = rational_detector<R>;
        using removed_rational_L = typename RDL::removed_rational_type;
        using removed_rational_R = typename RDR::removed_rational_type;
        using common_type = common_signed_t<removed_rational_L, removed_rational_R>;
        using L_type = conditional_op_t<RDL::value, rational, common_type>;
        using R_type = conditional_op_t<RDR::value, rational, common_type>;
    };

    template<class T, class A>
    [[nodiscard]] constexpr rgba_color<T> make_rgba_color(T r, T g, T b, A a) noexcept
    {
        return
        {
            .r{ r },
            .g{ g },
            .b{ b },
            .a{ color_tint_cast<T>(a) }
        };
    }

    template<class L, class R, class Op2>
    [[nodiscard]] constexpr decltype(auto) upgrade_op2(const L& left, const R& right, Op2 op2) noexcept
    {
        using decuctor = common_rational_deductor<L, R>;

        return op2
        (
            color_tint_cast<typename decuctor::L_type>(left),
            color_tint_cast<typename decuctor::R_type>(right)
        );
    }


    template<class L, class R, class Op2>
    [[nodiscard]] constexpr decltype(auto) universal_op2
    (
        const rgba_color<L>& left,
        const R& right,
        Op2 op2
    ) noexcept
    {
        if constexpr (is_rgba_color_v<R>)
        {
            D_ASSERT(left.a == right.a);

            return make_rgba_color
            (
                upgrade_op2(left.r, right.r, op2),
                upgrade_op2(left.g, right.g, op2),
                upgrade_op2(left.b, right.b, op2),
                left.a
            );
        }
        else
        {
            return make_rgba_color
            (
                upgrade_op2(left.r, right, op2),
                upgrade_op2(left.g, right, op2),
                upgrade_op2(left.b, right, op2),
                left.a
            );
        }
    }

    struct plus
    {
        template<class L, class R>
        [[nodiscard]] constexpr decltype(auto) operator () (const L& left, const R& right) const noexcept
        {
            return left + right;
        }
    };

    struct minus
    {
        template<class L, class R>
        [[nodiscard]] constexpr decltype(auto) operator () (const L& left, const R& right) const noexcept
        {
            return left - right;
        }
    };

    struct multiplies
    {
        template<class L, class R>
        [[nodiscard]] constexpr decltype(auto) operator () (const L& left, const R& right) const noexcept
        {
            return left * right;
        }
    };

    struct divides
    {
        template<class L, class R>
        [[nodiscard]] constexpr decltype(auto) operator () (const L& left, const R& right) const noexcept
        {
            if constexpr (std::is_integral_v<L> && std::is_integral_v<R>)
            {
                using common_t = common_signed_t<L, R>;

                return simplify
                (
                    narrow<common_t>(left),
                    narrow<common_t>(right)
                );
            }
            else
            {
                return left / right;
            }
        }
    };
}

template<class L, class R>
[[nodiscard]] constexpr decltype(auto) operator + (const rgba_color<L>& left, const rgba_color<R>& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, plus{});
}

template<class L, class R>
[[nodiscard]] constexpr decltype(auto) operator - (const rgba_color<L>& left, const rgba_color<R>& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, minus{});
}

template<class L, class R, std::enable_if_t<std::disjunction_v<std::is_arithmetic<R>, is_rational<R>>, int> = 0>
[[nodiscard]] constexpr decltype(auto) operator * (const rgba_color<L>& left, const R& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, multiplies{});
}

template<class L, class R, std::enable_if_t<std::disjunction_v<std::is_arithmetic<L>, is_rational<L>>, int> = 0>
[[nodiscard]] constexpr decltype(auto) operator * (const L& left, const rgba_color<R>& right) noexcept
{
    return right * left;
}

template<class L, class R, std::enable_if_t<std::disjunction_v<std::is_arithmetic<R>, is_rational<R>>, int> = 0>
[[nodiscard]] constexpr decltype(auto) operator / (const rgba_color<L>& left, const R& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, divides{});
}


namespace color_literals
{
    [[nodiscard]]
    constexpr rgba_color32_t operator "" _rgb(unsigned long long rgb) noexcept
    {
        return u32rgb_to_color(narrow<u32argb_t>(rgb));
    }

    [[nodiscard]]
    constexpr rgba_color32_t operator "" _argb(unsigned long long argb) noexcept
    {
        return u32argb_to_color(narrow<u32argb_t>(argb));
    }

    [[nodiscard]]
    constexpr rgba_colorf_t operator "" _rgbf(unsigned long long rgb) noexcept
    {
        return to_colorf(u32rgb_to_color(narrow<u32argb_t>(rgb)));
    }

    [[nodiscard]]
    constexpr rgba_colorf_t operator "" _argbf(unsigned long long argb) noexcept
    {
        return to_colorf(u32argb_to_color(narrow<u32argb_t>(argb)));
    }
}

namespace colors
{
    using namespace color_literals;

    constexpr auto black = 0x000000_rgb;
    constexpr auto gray = 0x7f7f7f_rgb;
    constexpr auto white = 0xffffff_rgb;

    constexpr auto red = 0xff0000_rgb;
    constexpr auto green = 0x00ff00_rgb;
    constexpr auto blue = 0x0000ff_rgb;

    constexpr auto cyan = 0x00ffff_rgb;
    constexpr auto magenta = 0xff00ff_rgb;
    constexpr auto yellow = 0xffff00_rgb;

    constexpr auto black_f = to_colorf(black);
    constexpr auto gray_f = to_colorf(gray);
    constexpr auto white_f = to_colorf(white);

    constexpr auto red_f = to_colorf(red);
    constexpr auto green_f = to_colorf(green);
    constexpr auto blue_f = to_colorf(blue);

    constexpr auto cyan_f = to_colorf(cyan);
    constexpr auto magenta_f = to_colorf(magenta);
    constexpr auto yellow_f = to_colorf(yellow);
}