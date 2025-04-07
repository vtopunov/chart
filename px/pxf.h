#pragma once

#include <px/fwd.h>


namespace px
{
    namespace private_detail_is_safe_conversion_pxf
    {
        template<class PXF, class PX, class Source>
        [[nodiscard]] constexpr bool is_safe_conversion_pxf_impl(const Source& v) noexcept
        {
            static_assert(std::is_floating_point_v<PXF>);
            static_assert(std::is_integral_v<PX>);
            static_assert(std::is_signed_v<PX>);

            constexpr auto gl_digits = numeric_digits_v<PXF>;
            constexpr auto px_digits = numeric_digits_v<PX>;
            constexpr auto glpx_digits = std::min(gl_digits, px_digits);
            constexpr auto source_digits = numeric_digits_v<Source>;

            if constexpr (std::is_floating_point_v<Source> || (glpx_digits < source_digits))
            {
                constexpr auto px_max = numeric_max_v<PX>;
                constexpr auto glpx_max = px_max >> (px_digits - glpx_digits);
                constexpr auto glpx_max_source = static_cast<Source>(glpx_max);

                if constexpr (std::is_unsigned_v<Source>)
                {
                    return v <= glpx_max_source;
                }
                else
                {
                    constexpr auto glpx_lowest = -glpx_max;
                    constexpr auto glpx_lowest_source = static_cast<Source>(glpx_lowest);
                    return (v >= glpx_lowest_source)
                        && (v <= glpx_max_source);
                }
            }
            else
            {
                return true;
            }
        }
    }

    template<class Source>
    [[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<Source>, bool> is_safe_conversion_pxf(const Source& v) noexcept
    {
        return private_detail_is_safe_conversion_pxf::is_safe_conversion_pxf_impl<npxf_t, pxoff_t>(v);
    }


    template<class T>
    [[nodiscard]] constexpr auto md_is_safe_conversion_pxf(const T& v) noexcept -> decltype(is_safe_conversion_pxf(v))
    {
        return is_safe_conversion_pxf(v);
    }

    template<class V>
    [[nodiscard]] constexpr auto md_is_safe_conversion_pxf(const V& v) noexcept -> decltype(md_is_safe_conversion_pxf(as_vec2(v)._0))
    {
        return md_is_safe_conversion_pxf(v._0)
            && md_is_safe_conversion_pxf(v._1);
    }

    template<class R>
    [[nodiscard]] constexpr auto md_is_safe_conversion_pxf(const R& v) noexcept ->
        decltype(md_is_safe_conversion_pxf(as_rectangle(v).position) && md_is_safe_conversion_pxf(as_rectangle(v).sizes))
    {
        return is_safe_conversion_pxf(v.position)
            && is_safe_conversion_pxf(v.sizes);
    }


    template<class T, class U>
    [[nodiscard]] constexpr bool update_pxf(T& value, U&& new_value) noexcept
    {
        const auto ok = (new_value != value) && md_is_safe_conversion_pxf(new_value);
        if (ok) [[likely]]
        {
            value = std::forward<U>(new_value);
        }

        return ok;
    }

    template<class T>
    using decl_is_safe_conversion_pxf_t = decltype(is_safe_conversion_pxf(std::declval<const T&>()));

    template<class T>
    [[nodiscard]] constexpr enable_if_detected_and_t<npxf_t, decl_is_safe_conversion_pxf_t, T> to_pxf(const T& value) noexcept
    {
        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
        D_ASSERT(is_safe_conversion_pxf(value));
        return static_cast<npxf_t>(value);
        D_WARNING_POP;
    }

    template<class T>
    using decl_to_pxf_t = decltype(to_pxf(std::declval<const T&>()));

    template<class Target, class Source>
    [[nodiscard]] constexpr Target narrow_px(const Source& v) noexcept
    {
        if constexpr (is_same_uncvref_v<
            Target, detected_or_t<std::type_identity<Target>, decl_to_pxf_t, Source>
        >)
        {
            return to_pxf(v);
        }
        else
        {
            return narrow<Target>(v);
        }
    }
}
