#pragma once

#include <cmath>
#include <functional>

#include <core/rational.h>

using argb_uint32_t = uint32_t;
static_assert( sizeof(argb_uint32_t) == 4 && std::is_unsigned_v<argb_uint32_t> );

using argb_tint_byte_t = uint8_t;
static_assert( sizeof(argb_tint_byte_t) == 1 && std::is_unsigned_v<argb_tint_byte_t> );

template<class T>
constexpr T argb_tint_max() noexcept
{
    if constexpr (std::is_floating_point_v<T>)
    {
        return static_cast<T>( 1.0 );
    }
    else
    {
        constexpr auto byte_max = std::numeric_limits<argb_tint_byte_t>::max();

        if constexpr (is_rational_v<T>)
        {
            return rational_cast<T>( byte_max );
        }
        else
        {
            return narrow_cast<T>( byte_max );
        }
    }
}

template<class T>
struct upgrade_tint_type
{
    using type = upgrade_int_t<T>;
};

template<class T>
struct upgrade_tint_type<rational<T>>
{
    using type = rational<upgrade_int_t<T>>;
};

template<class T>
using upgrade_tint_type_t = typename upgrade_tint_type<T>::type;

template<class Target, class Source>
constexpr Target color_cast(const Source& src) noexcept;

template<class T>
struct argb_color
{
    using tint_type = T;

    tint_type b;
    tint_type g;
    tint_type r;
    tint_type a;

    constexpr auto operator<=>(const argb_color&) const noexcept = default;

    using upgrade_tint_type = upgrade_tint_type_t<tint_type>;

    template<
        bool enable_bool = true,
        class = std::enable_if_t<( !std::is_same_v<tint_type, upgrade_tint_type>&& enable_bool )>
    > constexpr operator argb_color<upgrade_tint_type>() const noexcept
    {
        return color_cast<argb_color<upgrade_tint_type>>(*this);
    }

    static constexpr argb_color instance(tint_type a, tint_type r, tint_type g, tint_type b) noexcept
    {
        return {b, g, r, a};
    }

    static constexpr argb_color instance(tint_type r, tint_type g, tint_type b) noexcept
    {
        return instance(argb_tint_max<tint_type>(), r, g, b);
    }
};

using argb_color32_t = argb_color<argb_tint_byte_t>;

static_assert( sizeof(argb_color32_t) == 4 );

#pragma warning(push)
#pragma warning(disable : 26472) //  Don't use a static_cast for arithmetic conversions. Use brace initialization, narrow_cast or narrow

constexpr argb_tint_byte_t a_argb_byte(argb_uint32_t argb) noexcept
{
    return static_cast<argb_tint_byte_t>( argb >> 24u );
}

constexpr argb_tint_byte_t r_argb_byte(argb_uint32_t argb) noexcept
{
    return static_cast<argb_tint_byte_t>( ( argb >> 16u ) & 0xffu );
}

constexpr argb_tint_byte_t g_argb_byte(argb_uint32_t argb) noexcept
{
    return static_cast<argb_tint_byte_t>( ( argb >> 8u ) & 0xffu );
}

constexpr argb_tint_byte_t b_argb_byte(argb_uint32_t argb) noexcept
{
    return static_cast<argb_tint_byte_t>( argb & 0xffu );
}

#pragma warning(pop)

constexpr argb_color32_t argb_uint_to_color32(argb_uint32_t argb) noexcept
{
    return argb_color32_t::instance
    (
        a_argb_byte(argb),
        r_argb_byte(argb),
        g_argb_byte(argb),
        b_argb_byte(argb)
    );
}

constexpr argb_color32_t rgb_uint_to_color32(argb_uint32_t rgb) noexcept
{
    D_ASSERT(!a_argb_byte(rgb));
    return argb_color32_t::instance
    (
        r_argb_byte(rgb),
        g_argb_byte(rgb),
        b_argb_byte(rgb)
    );
}

constexpr argb_uint32_t color_to_argb_uint32(argb_color32_t color) noexcept
{
    return static_cast<argb_uint32_t>( color.a ) << 24
        | static_cast<argb_uint32_t>( color.r ) << 16
        | static_cast<argb_uint32_t>( color.g ) << 8
        | static_cast<argb_uint32_t>( color.b );
}

template <class T>
struct is_argb_color : public std::false_type
{};

template <class T>
struct is_argb_color<argb_color<T>> : public std::true_type
{};

template<class T>
inline constexpr bool is_argb_color_v = is_argb_color<T>::value;

template<class Target, class Source>
constexpr Target argb_color_tint_cast(Source source) noexcept
{
    if constexpr (std::is_floating_point_v<Target>)
    {
        constexpr auto target_max = argb_tint_max<Target>();

        if constexpr (is_rational_v<Source>)
        {
            using rational_int_type_t = typename Source::int_type;
            constexpr auto source_max = argb_tint_max<rational_int_type_t>();
            return target_max * rational_cast<Target>( source / source_max );
        }
        else
        {
            if constexpr (std::is_integral_v<Source>)
            {
                constexpr auto source_max = argb_tint_max<Source>();
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


template<class Target, class Source>
constexpr Target color_cast(const Source& src) noexcept
{
    if constexpr (is_argb_color_v<Target>)
    {
        using target_tint_t = typename Target::tint_type;

        if constexpr (is_argb_color_v<Source>)
        {
            using source_tint_t = typename Source::tint_type;

            constexpr auto tint_cast = [] (source_tint_t tint) noexcept
            {
                return argb_color_tint_cast<target_tint_t>( tint );
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
            const auto argb_color32 = argb_uint_to_color32(argb_uint32);
            return color_cast<Target>( argb_color32 );
        }
    }
    else
    {
        static_assert( std::is_integral_v<Target> && is_argb_color_v<Source> );
        const auto argb_color32 = color_cast<argb_color32_t>( src );
        const auto argb_uint32 = color_to_argb_uint32(argb_color32);
        return narrow_cast<Target>( argb_uint32 );
    }
}


namespace private_detail_argb_color
{
    template<class T, class A>
    constexpr argb_color<T> make_argb_color(A a, T r, T g, T b) noexcept
    {
        return argb_color<T>::instance(argb_color_tint_cast<T>(a), r, g, b);
    }

    template<class L, class R, class Op2>
    constexpr decltype( auto ) upgrade_op2(const L& left, const R& right, Op2 op2) noexcept
    {
        return op2
        (
            argb_color_tint_cast<upgrade_tint_type_t<L>>( left ),
            argb_color_tint_cast<upgrade_tint_type_t<R>>( right )
        );
    }

    template<class L, class R, class Op2>
    constexpr decltype( auto ) universal_op2
    (
        const argb_color<L>& left,
        const argb_color<R>& right,
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

    template<class L, class R, class Op2>
    constexpr decltype( auto ) universal_op2
    (
        const argb_color<L>& left,
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
        template<class L, class R>
        constexpr decltype( auto ) operator () (const L& left, const R& right) const noexcept
        {
            return left + right;
        }
    };

    struct minus
    {
        template<class L, class R>
        constexpr decltype( auto ) operator () (const L& left, const R& right) const noexcept
        {
            return left - right;
        }
    };

    struct multiplies
    {
        template<class L, class R>
        constexpr decltype( auto ) operator () (const L& left, const R& right) const noexcept
        {
            return left * right;
        }
    };

    struct divides
    {
        template<class L, class R>
        constexpr decltype( auto ) operator () (const L& left, const R& right) const noexcept
        {
            if constexpr (std::is_integral_v<L> && std::is_integral_v<R>)
            {
                using common_type = std::common_type_t<upgrade_tint_type_t<L>, upgrade_tint_type_t<R>>;

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


template<class L, class R>
constexpr decltype( auto ) operator + (const argb_color<L>& left, const argb_color<R>& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, plus{});
}

template<class L, class R>
constexpr decltype( auto ) operator - (const argb_color<L>& left, const argb_color<R>& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, minus{});
}

template<class L, class R, class = std::enable_if_t< std::is_integral_v<R> || is_rational_v<R> > >
constexpr decltype( auto ) operator * (const argb_color<L>& left, const R& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, multiplies{});
}

template<class L, class R, class = std::enable_if_t< std::is_integral_v<L> || is_rational_v<L> > >
constexpr decltype( auto ) operator * (const L& left, const argb_color<R>& right) noexcept
{
    return right * left;
}

template<class L, class R, class = std::enable_if_t< std::is_integral_v<R> || is_rational_v<R> > >
constexpr decltype( auto ) operator / (const argb_color<L>& left, const R& right) noexcept
{
    using namespace private_detail_argb_color;
    return universal_op2(left, right, divides{});
}

namespace color_literals
{
    constexpr argb_color32_t operator "" _rgb(unsigned long long rgb) noexcept
    {
        return rgb_uint_to_color32(narrow_cast<argb_uint32_t>( rgb ));
    }

    constexpr argb_color32_t operator "" _argb(unsigned long long argb) noexcept
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
}