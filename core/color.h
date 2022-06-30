#pragma once

#include <compare>

#include <core/colorfwd.h>
#include <core/rational.h>

template<class T> [[nodiscard]]
constexpr T tint_max() noexcept
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
            return narrow_cast<T>(byte_max);
        }
    }
}

template<class Target, class Source> [[nodiscard]]
constexpr Target color_cast(const Source& src) noexcept;

template<class T>
struct rgba_color
{
    using tint_type = T;
    using view_type = span<const tint_type, 4u>;

    tint_type r;
    tint_type g;
    tint_type b;
    tint_type a;

    [[nodiscard]]
    constexpr auto operator<=>(const rgba_color&) const noexcept = default;

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
        static_assert(sizeof(rgba_color<T>) == 4u * sizeof(T));
        return view_type{ std::addressof(r), view_type::extent };
    }

    [[nodiscard]]
    static constexpr rgba_color instance(tint_type a, tint_type r, tint_type g, tint_type b) noexcept
    {
        return { r, g, b, a };
    }

    [[nodiscard]]
    static constexpr rgba_color instance(view_type view) noexcept
    {
        return instance(view[0], view[1], view[2], view[3]);
    }

    [[nodiscard]]
    static constexpr rgba_color instance(tint_type r, tint_type g, tint_type b) noexcept
    {
        return instance(tint_max<tint_type>(), r, g, b);
    }
};

using rgba_color32_t = rgba_color<u8tint_t>;
using rgba_color32_view = rgba_color32_t::view_type;
using rgba_colorf_t = rgba_color<float>;
using rgba_colorf_view = rgba_colorf_t::view_type;

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
    return rgba_color32_t::instance
    (
        a_argb32(argb),
        r_argb32(argb),
        g_argb32(argb),
        b_argb32(argb)
    );
}

[[nodiscard]]
constexpr rgba_color32_t u32rgb_to_color(u32argb_t rgb) noexcept
{
    D_ASSERT(!a_argb32(rgb));
    return rgba_color32_t::instance
    (
        r_argb32(rgb),
        g_argb32(rgb),
        b_argb32(rgb)
    );
}

[[nodiscard]]
constexpr u32argb_t color32_to_uint(rgba_color32_t color) noexcept
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

template<class Target, class Source> [[nodiscard]]
constexpr Target color_tint_cast(Source source) noexcept
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
                constexpr auto ratio = target_max / narrow_cast<Target>(source_max);
                return ratio * narrow_cast<Target>(source);
            }
            else
            {
                return narrow_cast<Target>(source);
            }
        }
    }
    else
    {
        constexpr auto target_is_integral = std::is_integral_v<Target>;
        constexpr auto source_is_integral = std::is_integral_v<Source>;

        if constexpr (target_is_integral && source_is_integral)
        {
            return narrow_cast<Target>(source);
        }
        else
        {
            static_assert(target_is_integral || is_rational_v<Target>);
            static_assert(source_is_integral || is_rational_v<Source>);
            return rational_cast<Target>(source);
        }
    }
}


template<class Target, class Source> [[nodiscard]]
constexpr Target color_cast(const Source& src) noexcept
{
    if constexpr (is_rgba_color_v<Target>)
    {
        using target_tint_t = typename Target::tint_type;

        if constexpr (is_rgba_color_v<Source>)
        {
            using source_tint_t = typename Source::tint_type;

            constexpr auto tint_cast = [](source_tint_t tint) noexcept
            {
                return color_tint_cast<target_tint_t>(tint);
            };

            return Target::instance
            (
                tint_cast(src.a),
                tint_cast(src.r),
                tint_cast(src.g),
                tint_cast(src.b)
            );
        }
        else
        {
            static_assert(std::is_integral_v<Source>);
            const auto argb_uint32 = narrow_cast<u32argb_t>(src);
            const auto rgba_color32 = u32argb_to_color(argb_uint32);
            return color_cast<Target>(rgba_color32);
        }
    }
    else
    {
        static_assert(std::is_integral_v<Target> && is_rgba_color_v<Source>);
        const auto rgba_color32 = color_cast<rgba_color32_t>(src);
        const auto argb_uint32 = color32_to_uint(rgba_color32);
        return narrow_cast<Target>(argb_uint32);
    }
}

template<class T> [[nodiscard]]
constexpr rgba_color<T> inverse(const rgba_color<T>& c) noexcept
{
    constexpr auto tint_max_c = tint_max<T>();

    return rgba_color<T>::instance
    (
        tint_max_c,
        tint_max_c - c.r,
        tint_max_c - c.g,
        tint_max_c - c.b
    );
}

namespace private_detail_argb_color
{
    template<class T>
    using signed_int0_t = std::conditional_t<
        is_narrowing_or_same_v<T, ptrdiff_t>, ptrdiff_t,
        std::conditional_t<is_narrowing_or_same_v<T, int64_t>, int64_t, intmax_t>
    >;

    template<class T>
    using signed_int_t = conditional_op_t<std::is_integral_v<T>, signed_int0_t, T>;

    template<class L, class R>
    using common_signed_int_t = std::common_type_t<signed_int_t<L>, signed_int_t<R>>;

    template<class L, class R>
    struct common_rational_deductor
    {
        using RDL = rational_detector<L>;
        using RDR = rational_detector<R>;
        using removed_rational_L = typename RDL::removed_rational_type;
        using removed_rational_R = typename RDR::removed_rational_type;
        using common_type = common_signed_int_t<removed_rational_L, removed_rational_R>;
        using L_type = conditional_op_t<RDL::value, rational, common_type>;
        using R_type = conditional_op_t<RDR::value, rational, common_type>;
    };

    template<class T, class A>
    [[nodiscard]] constexpr rgba_color<T> make_argb_color(A a, T r, T g, T b) noexcept
    {
        return rgba_color<T>::instance(color_tint_cast<T>(a), r, g, b);
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

            return make_argb_color
            (
                left.a,
                upgrade_op2(left.r, right.r, op2),
                upgrade_op2(left.g, right.g, op2),
                upgrade_op2(left.b, right.b, op2)
            );
        }
        else
        {
            return make_argb_color
            (
                left.a,
                upgrade_op2(left.r, right, op2),
                upgrade_op2(left.g, right, op2),
                upgrade_op2(left.b, right, op2)
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
                using common_t = common_signed_int_t<L, R>;

                return simplify
                (
                    narrow_cast<common_t>(left),
                    narrow_cast<common_t>(right)
                );
            }
            else
            {
                return left / right;
            }
        }
    };
}


template<class L, class R> [[nodiscard]]
constexpr decltype(auto) operator + (const rgba_color<L>& left, const rgba_color<R>& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, plus{});
}

template<class L, class R> [[nodiscard]]
constexpr decltype(auto) operator - (const rgba_color<L>& left, const rgba_color<R>& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, minus{});
}

template<class L, class R, class = std::enable_if_t<std::disjunction_v<std::is_integral<R>, is_rational<R>> > > [[nodiscard]]
constexpr decltype(auto) operator * (const rgba_color<L>& left, const R& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, multiplies{});
}

template<class L, class R, class = std::enable_if_t<std::disjunction_v<std::is_integral<R>, is_rational<R>> > > [[nodiscard]]
constexpr decltype(auto) operator * (const L& left, const rgba_color<R>& right) noexcept
{
    return right * left;
}

template<class L, class R, class = std::enable_if_t<std::disjunction_v<std::is_integral<R>, is_rational<R>> > > [[nodiscard]]
constexpr decltype(auto) operator / (const rgba_color<L>& left, const R& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, divides{});
}

namespace color_literals
{
    [[nodiscard]]
    constexpr rgba_color32_t operator "" _rgb(unsigned long long rgb) noexcept
    {
        return u32rgb_to_color(narrow_cast<u32argb_t>(rgb));
    }

    [[nodiscard]]
    constexpr rgba_color32_t operator "" _argb(unsigned long long argb) noexcept
    {
        return u32argb_to_color(narrow_cast<u32argb_t>(argb));
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
}