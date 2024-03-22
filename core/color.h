#pragma once

#include <core/rational.h>


constexpr auto byte_tint_max = numeric_max_v<byte_tint_t>;

namespace private_detail_argb_color
{
    template<class T>
    struct tint_max_impl_t
    {
        constexpr operator T () const noexcept
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                return static_cast<T>(1.0);
            }
            else
            {
                if constexpr (is_rational_v<T>)
                {
                    return rational_cast<T>(byte_tint_max);
                }
                else
                {
                    static_assert(std::is_integral_v<T>);
                    return narrow<T>(byte_tint_max);
                }
            }
        }
    };
}

template<class T = void>
struct tint_max_t : private_detail_argb_color::tint_max_impl_t<T>
{
    static_assert(!std::is_reference_v<T>);

    [[nodiscard]] constexpr T operator () () const noexcept
    {
        return tint_max_t::operator T();
    }
};

template<>
struct tint_max_t<void>
{
    template<class T, std::enable_if_t<std::negation_v<std::is_reference<T>>, int> = 0>
    constexpr operator T () const noexcept
    {
        constexpr private_detail_argb_color::tint_max_impl_t<T> impl{};
        return impl.operator T();
    }
};

template<class T = void>
constexpr tint_max_t<T> tint_max_v{};


template<class Target, class Source>
[[nodiscard]] constexpr Target color_cast(const Source& src) noexcept;

constexpr size_t rgba_color_view_extent{ 4u };

template<class T>
using rgba_color_view = span<const T, rgba_color_view_extent>;

template<class T>
struct rgba_color
{
    using tint_type = T;
    using value_type = tint_type;
    using view_type = rgba_color_view<tint_type>;
    static constexpr auto extent = rgba_color_view_extent;

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

template<class T, class A>
[[nodiscard]] constexpr rgba_color<std::remove_cvref_t<T>> make_rgba(T&& r, T&& g, T&& b, A&& a) noexcept
{
    return
    {
        .r{ std::forward<T>(r) },
        .g{ std::forward<T>(g) },
        .b{ std::forward<T>(b) },
        .a{ std::forward<A>(a) }
    };
}

template<class T>
[[nodiscard]] constexpr rgba_color<std::remove_cvref_t<T>> make_rgb(T&& r, T&& g, T&& b) noexcept
{
    return make_rgba
    (
        std::forward<T>(r),
        std::forward<T>(g),
        std::forward<T>(b),
        tint_max_v<>
    );
}

template<class T>
[[nodiscard]] constexpr rgba_color<T> make_white(const T& value) noexcept
{
    return make_rgb(value, value, value);
}


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast)

[[nodiscard]]
constexpr byte_tint_t a_argb(argb_t argb) noexcept
{
    return static_cast<byte_tint_t>(argb >> 24);
}

[[nodiscard]]
constexpr byte_tint_t r_argb(argb_t argb) noexcept
{
    return static_cast<byte_tint_t>((argb >> 16) & 0xffu);
}

[[nodiscard]]
constexpr byte_tint_t g_argb(argb_t argb) noexcept
{
    return static_cast<byte_tint_t>((argb >> 8) & 0xffu);
}

[[nodiscard]]
constexpr byte_tint_t b_argb(argb_t argb) noexcept
{
    return static_cast<byte_tint_t>(argb & 0xffu);
}

D_WARNING_POP


[[nodiscard]]
constexpr rgba_color_t argb_to_color(argb_t argb) noexcept
{
    return
    {
        .r{ r_argb(argb) },
        .g{ g_argb(argb) },
        .b{ b_argb(argb) },
        .a{ a_argb(argb) }
    };
}

[[nodiscard]]
constexpr rgba_color_t rgb_to_color(argb_t rgb) noexcept
{
    D_ASSERT(!a_argb(rgb));
    return make_rgb
    (
        r_argb(rgb),
        g_argb(rgb),
        b_argb(rgb)
    );
}

[[nodiscard]]
constexpr argb_t color_to_argb(rgba_color_t color) noexcept
{
    return static_cast<argb_t>(color.a) << 24
        | static_cast<argb_t>(color.r) << 16
        | static_cast<argb_t>(color.g) << 8
        | static_cast<argb_t>(color.b);
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
        constexpr auto target_max = tint_max_v<Target>();

        if constexpr (is_rational_v<Source>)
        {
            using rational_int_type_t = typename Source::int_type;
            constexpr auto source_max = tint_max_v<rational_int_type_t>();
            return target_max * rational_cast<Target>(source / source_max);
        }
        else
        {
            if constexpr (std::is_integral_v<Source>)
            {
                constexpr auto source_max = tint_max_v<Source>();
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

        constexpr auto tint_cast = [] (auto tint) noexcept
        {
            return color_tint_cast<target_tint_t>(tint);
        };

        if constexpr (is_rgba_color_v<Source>)
        {
            return
            {
                .r{ tint_cast(src.r) },
                .g{ tint_cast(src.g) },
                .b{ tint_cast(src.b) },
                .a{ tint_cast(src.a) }
            };
        }
        else if constexpr (rgba_color_view_extent == extent_v<Source>)
        {
            return
            {
                .r{ tint_cast(src[0_uz])},
                .g{ tint_cast(src[1_uz])},
                .b{ tint_cast(src[2_uz])},
                .a{ tint_cast(src[3_uz])}
            };
        }
        else
        {
            static_assert(std::is_integral_v<Source>);
            const auto argb = narrow<argb_t>(src);
            const auto color = argb_to_color(argb);
            return color_cast<Target>(color);
        }
    }
    else
    {
        static_assert(std::is_integral_v<Target>);
        const auto color = color_cast<rgba_color_t>(src);
        const auto argb = color_to_argb(color);
        return narrow<Target>(argb);
    }
}

template<class T>
[[nodiscard]] constexpr rgba_color_t to_color(const rgba_color<T>& source) noexcept
{
    return color_cast<rgba_color_t>(source);
}

template<class T>
[[nodiscard]] constexpr rgbaf_color_t to_colorf(const rgba_color<T>& source) noexcept
{
    return color_cast<rgbaf_color_t>(source);
}

template<class T>
[[nodiscard]] constexpr T inverse_tint(const T& tint) noexcept
{
    constexpr auto tint_max_c = tint_max_v<T>();
    return static_cast<T>(tint_max_c - tint);
}

template<class T>
[[nodiscard]] constexpr rgba_color<T> inverse(const rgba_color<T>& c) noexcept
{
    return
    {
        .r{ inverse_tint(c.r) },
        .g{ inverse_tint(c.g) },
        .b{ inverse_tint(c.b) },
        .a{ c.a }
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
    constexpr byte_tint_t operator "" _tint(unsigned long long value) noexcept
    {
        return narrow<byte_tint_t>(value);
    }

    [[nodiscard]]
    constexpr float_tint_t operator "" _tint(long double value) noexcept
    {
        return static_cast<float_tint_t>(value);
    }

    [[nodiscard]]
    constexpr rgba_color_t operator "" _rgb(unsigned long long rgb) noexcept
    {
        return rgb_to_color(narrow<argb_t>(rgb));
    }

    [[nodiscard]]
    constexpr rgba_color_t operator "" _argb(unsigned long long argb) noexcept
    {
        return argb_to_color(narrow<argb_t>(argb));
    }

    [[nodiscard]]
    constexpr rgbaf_color_t operator "" _rgbf(unsigned long long rgb) noexcept
    {
        return to_colorf(rgb_to_color(narrow<argb_t>(rgb)));
    }

    [[nodiscard]]
    constexpr rgbaf_color_t operator "" _argbf(unsigned long long argb) noexcept
    {
        return to_colorf(argb_to_color(narrow<argb_t>(argb)));
    }
}

namespace colors
{
    using namespace color_literals;

    constexpr auto white = make_white(byte_tint_max);
    constexpr auto gray = make_white(0x7f_tint);
    constexpr auto black = inverse(white);

    constexpr auto red = black.with_red(tint_max_v<>);
    constexpr auto green = black.with_green(tint_max_v<>);
    constexpr auto blue = black.with_blue(tint_max_v<>);

    constexpr auto cyan = inverse(red);
    constexpr auto magenta = inverse(green);
    constexpr auto yellow = inverse(blue);

    constexpr auto white_f = to_colorf(white);
    constexpr auto gray_f = make_white(0.5_tint);
    constexpr auto black_f = to_colorf(black);

    constexpr auto red_f = to_colorf(red);
    constexpr auto green_f = to_colorf(green);
    constexpr auto blue_f = to_colorf(blue);

    constexpr auto cyan_f = to_colorf(cyan);
    constexpr auto magenta_f = to_colorf(magenta);
    constexpr auto yellow_f = to_colorf(yellow);
}