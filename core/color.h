#pragma once

#include <cmath>
#include <functional>
#include <span>

#include <core/rational.h>

template<class T>
inline constexpr bool is_uint_v = std::is_integral_v<T> && std::is_unsigned_v<T>;

using argb_uint32_t = uint32_t;
static_assert( sizeof(argb_uint32_t) == 4 && is_uint_v<argb_uint32_t> );

using tint_byte_t = uint8_t;
static_assert( sizeof(tint_byte_t) == 1 && is_uint_v<tint_byte_t> );

template<class T> [[nodiscard]]
constexpr T tint_max() noexcept
{
    if constexpr (std::is_floating_point_v<T>)
    {
        return static_cast<T>( 1.0 );
    }
    else
    {
        constexpr auto byte_max = std::numeric_limits<tint_byte_t>::max();

        if constexpr (is_rational_v<T>)
        {
            return rational_cast<T>( byte_max );
        }
        else
        {
            static_assert( std::is_integral_v<T> );
            return narrow_cast<T>( byte_max );
        }
    }
}

template<class T>
struct wide_tint_type
{
    using type = wide_int_t<T>;
};

template<class T>
struct wide_tint_type<rational<T>>
{
    using type = rational<wide_int_t<T>>;
};

template<class T>
using wide_tint_type_t = typename wide_tint_type<T>::type;

template<class Target, class Source> [[nodiscard]]
constexpr Target color_cast(const Source& src) noexcept;

template<class T>
struct rgba_color
{
    using tint_type = T;
    using view_type = std::span<const tint_type, 4u>;

    tint_type r;
    tint_type g;
    tint_type b;
    tint_type a;

    [[nodiscard]]
    constexpr auto operator<=>(const rgba_color&) const noexcept = default;

    using wide_tint_type = wide_tint_type_t<tint_type>;
    using wide_argb_color_type = rgba_color<wide_tint_type>;

    template<
        bool dummy = true,
        class = std::enable_if_t<( !std::is_same_v<tint_type, wide_tint_type>&& dummy )>
    > [[nodiscard]]
    constexpr operator wide_argb_color_type() const noexcept
    {
        return color_cast<wide_argb_color_type>(*this);
    }

    [[nodiscard]]
    constexpr operator view_type() const noexcept
    {
        static_assert(sizeof(rgba_color<T>) == 4 * sizeof(T));
        return view_type{ std::addressof(r), view_type::extent };
    }

    [[nodiscard]]
    static constexpr rgba_color instance(tint_type a, tint_type r, tint_type g, tint_type b) noexcept
    {
        return {r, g, b, a};
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

using rgba_color32_t = rgba_color<tint_byte_t>;
using rgba_color32_view = rgba_color32_t::view_type;
using rgba_colorf_t = rgba_color<float>;
using rgba_colorf_view = rgba_colorf_t::view_type;

static_assert( sizeof(rgba_color32_t) == 4 );

#pragma warning(push)
#pragma warning(disable : 26472) //  Don't use a static_cast for arithmetic conversions. Use brace initialization, narrow_cast or narrow

[[nodiscard]]
constexpr tint_byte_t a_argb_byte(argb_uint32_t argb) noexcept
{
    return static_cast<tint_byte_t>( argb >> 24u );
}

[[nodiscard]]
constexpr tint_byte_t r_argb_byte(argb_uint32_t argb) noexcept
{
    return static_cast<tint_byte_t>( ( argb >> 16u ) & 0xffu );
}

[[nodiscard]]
constexpr tint_byte_t g_argb_byte(argb_uint32_t argb) noexcept
{
    return static_cast<tint_byte_t>( ( argb >> 8u ) & 0xffu );
}

[[nodiscard]]
constexpr tint_byte_t b_argb_byte(argb_uint32_t argb) noexcept
{
    return static_cast<tint_byte_t>( argb & 0xffu );
}

#pragma warning(pop)

[[nodiscard]]
constexpr rgba_color32_t argb_uint_to_color32(argb_uint32_t argb) noexcept
{
    return rgba_color32_t::instance
    (
        a_argb_byte(argb),
        r_argb_byte(argb),
        g_argb_byte(argb),
        b_argb_byte(argb)
    );
}

[[nodiscard]]
constexpr rgba_color32_t rgb_uint_to_color32(argb_uint32_t rgb) noexcept
{
    D_ASSERT(!a_argb_byte(rgb));
    return rgba_color32_t::instance
    (
        r_argb_byte(rgb),
        g_argb_byte(rgb),
        b_argb_byte(rgb)
    );
}

[[nodiscard]]
constexpr argb_uint32_t color_to_argb_uint32(rgba_color32_t color) noexcept
{
    return static_cast<argb_uint32_t>( color.a ) << 24u
        | static_cast<argb_uint32_t>( color.r ) << 16u
        | static_cast<argb_uint32_t>( color.g ) << 8u
        | static_cast<argb_uint32_t>( color.b );
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
inline constexpr bool is_rgba_color_v = is_rgba_color<T>::value;

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
            return target_max * rational_cast<Target>( source / source_max );
        }
        else
        {
            if constexpr (std::is_integral_v<Source>)
            {
                constexpr auto source_max = tint_max<Source>();
                constexpr auto ratio = static_cast<Target>( target_max / static_cast<Target>( source_max ) );
                return ratio * static_cast<Target>( source );
            }
            else
            {
                static_assert( std::is_floating_point_v<Source> );
                return static_cast<Target>( source );
            }
        }
    }
    else
    {
        constexpr auto target_is_integral = std::is_integral_v<Target>;
        constexpr auto source_is_integral = std::is_integral_v<Source>;

        if constexpr (target_is_integral && source_is_integral)
        {
            return narrow_cast<Target>( source );
        }
        else
        {
            static_assert( target_is_integral || is_rational_v<Target> );
            static_assert( source_is_integral || is_rational_v<Source> );
            return rational_cast<Target>( source );
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

            constexpr auto tint_cast = [] (source_tint_t tint) noexcept
            {
                return color_tint_cast<target_tint_t>( tint );
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
            static_assert( std::is_integral_v<Source> );
            const auto argb_uint32 = narrow_cast<argb_uint32_t>( src );
            const auto rgba_color32 = argb_uint_to_color32(argb_uint32);
            return color_cast<Target>( rgba_color32 );
        }
    }
    else
    {
        static_assert( std::is_integral_v<Target> && is_rgba_color_v<Source> );
        const auto rgba_color32 = color_cast<rgba_color32_t>( src );
        const auto argb_uint32 = color_to_argb_uint32(rgba_color32);
        return narrow_cast<Target>( argb_uint32 );
    }
}

template<class T>
constexpr rgba_colorf_t to_colorf(const rgba_color<T>& source) noexcept
{
    return color_cast<rgba_colorf_t>(source);
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
    template<class T, class A> [[nodiscard]]
    constexpr rgba_color<T> make_argb_color(A a, T r, T g, T b) noexcept
    {
        return rgba_color<T>::instance(color_tint_cast<T>(a), r, g, b);
    }

    template<class L, class R, class Op2> [[nodiscard]]
    constexpr decltype( auto ) upgrade_op2(const L& left, const R& right, Op2 op2) noexcept
    {
        return op2
        (
            color_tint_cast<wide_tint_type_t<L>>( left ),
            color_tint_cast<wide_tint_type_t<R>>( right )
        );
    }

    template<class L, class R, class Op2> [[nodiscard]]
    constexpr decltype( auto ) universal_op2
    (
        const rgba_color<L>& left,
        const rgba_color<R>& right,
        Op2 op2
    ) noexcept
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

    template<class L, class R, class Op2> [[nodiscard]]
    constexpr decltype( auto ) universal_op2
    (
        const rgba_color<L>& left,
        const R& right,
        Op2 op2
    ) noexcept
    {
        return make_argb_color
        (
            left.a,
            upgrade_op2(left.r, right, op2),
            upgrade_op2(left.g, right, op2),
            upgrade_op2(left.b, right, op2)
        );
    }

    struct plus
    {
        template<class L, class R> [[nodiscard]]
        constexpr decltype( auto ) operator () (const L& left, const R& right) const noexcept
        {
            return left + right;
        }
    };

    struct minus
    {
        template<class L, class R> [[nodiscard]]
        constexpr decltype( auto ) operator () (const L& left, const R& right) const noexcept
        {
            return left - right;
        }
    };

    struct multiplies
    {
        template<class L, class R> [[nodiscard]]
        constexpr decltype( auto ) operator () (const L& left, const R& right) const noexcept
        {
            return left * right;
        }
    };

    struct divides
    {
        template<class L, class R> [[nodiscard]]
        constexpr decltype( auto ) operator () (const L& left, const R& right) const noexcept
        {
            if constexpr (std::is_integral_v<L> && std::is_integral_v<R>)
            {
                using common_type = std::common_type_t<wide_tint_type_t<L>, wide_tint_type_t<R>>;

                return simplify
                (
                    narrow_cast<common_type>( left ),
                    narrow_cast<common_type>( right )
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
constexpr decltype( auto ) operator + (const rgba_color<L>& left, const rgba_color<R>& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, plus{});
}

template<class L, class R> [[nodiscard]]
constexpr decltype( auto ) operator - (const rgba_color<L>& left, const rgba_color<R>& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, minus{});
}

template<class L, class R, class = std::enable_if_t< std::is_integral_v<R> || is_rational_v<R> > > [[nodiscard]]
constexpr decltype( auto ) operator * (const rgba_color<L>& left, const R& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, multiplies{});
}

template<class L, class R, class = std::enable_if_t< std::is_integral_v<L> || is_rational_v<L> > > [[nodiscard]]
constexpr decltype( auto ) operator * (const L& left, const rgba_color<R>& right) noexcept
{
    return right * left;
}

template<class L, class R, class = std::enable_if_t< std::is_integral_v<R> || is_rational_v<R> > > [[nodiscard]]
constexpr decltype( auto ) operator / (const rgba_color<L>& left, const R& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, divides{});
}

namespace color_literals
{
    [[nodiscard]]
    constexpr rgba_color32_t operator "" _rgb(unsigned long long rgb) noexcept
    {
        return rgb_uint_to_color32(narrow_cast<argb_uint32_t>( rgb ));
    }

    [[nodiscard]]
    constexpr rgba_color32_t operator "" _argb(unsigned long long argb) noexcept
    {
        return argb_uint_to_color32(narrow_cast<argb_uint32_t>( argb ));
    }
}

namespace colors
{
    using namespace color_literals;

    inline constexpr auto black = 0x000000_rgb;
    inline constexpr auto gray = 0x808080_rgb;
    inline constexpr auto white = 0xffffff_rgb;

    inline constexpr auto red = 0xff0000_rgb;
    inline constexpr auto green = 0x00ff00_rgb;
    inline constexpr auto blue = 0x0000ff_rgb;

    inline constexpr auto cyan = 0x00ffff_rgb;
    inline constexpr auto magenta = 0xff00ff_rgb;
    inline constexpr auto yellow = 0xffff00_rgb;

    inline constexpr auto black_f = to_colorf(black);
    inline constexpr auto gray_f = to_colorf(gray);
    inline constexpr auto white_f = to_colorf(white);

    inline constexpr auto red_f = to_colorf(red);
    inline constexpr auto green_f = to_colorf(green);
    inline constexpr auto blue_f = to_colorf(blue);

    inline constexpr auto cyan_f = to_colorf(cyan);
    inline constexpr auto magenta_f = to_colorf(magenta);
    inline constexpr auto yellow_f = to_colorf(yellow);
}