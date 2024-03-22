#include <utility/shader_library.h>


namespace
{
    void test_conversion_glpx() noexcept
    {
        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_truncation_of_value);

        using private_detail_is_safe_conversion_glpx::is_safe_conversion_glpx_impl;

        {
            constexpr auto u_pxfloat_max = (numeric_max_v<uint32_t>) >> (numeric_digits_v<uint32_t> -numeric_digits_v<float>);
            constexpr int32_t s_pxfloat_max = u_pxfloat_max;

            static_assert(is_safe_conversion_glpx_impl<float, int32_t>(u_pxfloat_max));
            static_assert(is_safe_conversion_glpx_impl<float, int32_t>(s_pxfloat_max));
            static_assert(is_safe_conversion_glpx_impl<float, int32_t>(-s_pxfloat_max));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t>(u_pxfloat_max + 1u));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t>(s_pxfloat_max + 1));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t>(-s_pxfloat_max - 1));
            static_assert(is_safe_conversion_glpx_impl<float, int32_t, float>(u_pxfloat_max));
            static_assert(is_safe_conversion_glpx_impl<float, int32_t, float>(s_pxfloat_max));
            static_assert(is_safe_conversion_glpx_impl<float, int32_t, float>(-s_pxfloat_max));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t, float>(u_pxfloat_max + 1u));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t, float>(s_pxfloat_max + 1));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t, float>(-s_pxfloat_max - 1));
            static_assert(is_safe_conversion_glpx_impl<float, int32_t, double>(u_pxfloat_max));
            static_assert(is_safe_conversion_glpx_impl<float, int32_t, double>(s_pxfloat_max));
            static_assert(is_safe_conversion_glpx_impl<float, int32_t, double>(-s_pxfloat_max));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t, double>(u_pxfloat_max + 1u));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t, double>(s_pxfloat_max + 1));
            static_assert(!is_safe_conversion_glpx_impl<float, int32_t, double>(-s_pxfloat_max - 1));
        }

        {
            constexpr auto s_px16_max = numeric_max_v<int16_t>;
            constexpr uint16_t u_px16_max = s_px16_max;

            static_assert(is_safe_conversion_glpx_impl<float, int16_t>(u_px16_max));
            static_assert(is_safe_conversion_glpx_impl<float, int16_t>(s_px16_max));
            static_assert(is_safe_conversion_glpx_impl<float, int16_t>(-s_px16_max));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t>(u_px16_max + 1u));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t>(s_px16_max + 1));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t>(-s_px16_max - 1));
            static_assert(is_safe_conversion_glpx_impl<float, int16_t, float>(u_px16_max));
            static_assert(is_safe_conversion_glpx_impl<float, int16_t, float>(s_px16_max));
            static_assert(is_safe_conversion_glpx_impl<float, int16_t, float>(-s_px16_max));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t, float>(u_px16_max + 1u));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t, float>(s_px16_max + 1));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t, float>(-s_px16_max - 1));
            static_assert(is_safe_conversion_glpx_impl<float, int16_t, double>(u_px16_max));
            static_assert(is_safe_conversion_glpx_impl<float, int16_t, double>(s_px16_max));
            static_assert(is_safe_conversion_glpx_impl<float, int16_t, double>(-s_px16_max));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t, double>(u_px16_max + 1u));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t, double>(s_px16_max + 1));
            static_assert(!is_safe_conversion_glpx_impl<float, int16_t, double>(-s_px16_max - 1));
        }

        {
            constexpr auto u_pxdouble_max = (numeric_max_v<uint64_t>) >> (numeric_digits_v<uint64_t> -numeric_digits_v<double>);
            constexpr int64_t s_pxdouble_max = u_pxdouble_max;

            static_assert(is_safe_conversion_glpx_impl<double, int64_t>(u_pxdouble_max));
            static_assert(is_safe_conversion_glpx_impl<double, int64_t>(s_pxdouble_max));
            static_assert(is_safe_conversion_glpx_impl<double, int64_t>(-s_pxdouble_max));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t>(u_pxdouble_max + 1u));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t>(s_pxdouble_max + 1));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t>(-s_pxdouble_max - 1));
            static_assert(is_safe_conversion_glpx_impl<double, int64_t, float>(u_pxdouble_max));
            static_assert(is_safe_conversion_glpx_impl<double, int64_t, float>(s_pxdouble_max));
            static_assert(is_safe_conversion_glpx_impl<double, int64_t, float>(-s_pxdouble_max));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t, float>(u_pxdouble_max << 1));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t, float>(s_pxdouble_max << 1));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t, float>(-(s_pxdouble_max << 1)));
            static_assert(is_safe_conversion_glpx_impl<double, int64_t, double>(u_pxdouble_max));
            static_assert(is_safe_conversion_glpx_impl<double, int64_t, double>(s_pxdouble_max));
            static_assert(is_safe_conversion_glpx_impl<double, int64_t, double>(-s_pxdouble_max));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t, double>(u_pxdouble_max + 1u));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t, double>(s_pxdouble_max + 1));
            static_assert(!is_safe_conversion_glpx_impl<double, int64_t, double>(-s_pxdouble_max - 1));
        }

        {
            constexpr auto s_px32_max = numeric_max_v<int32_t>;
            constexpr uint32_t u_px32_max = s_px32_max;

            static_assert(is_safe_conversion_glpx_impl<double, int32_t>(u_px32_max));
            static_assert(is_safe_conversion_glpx_impl<double, int32_t>(s_px32_max));
            static_assert(is_safe_conversion_glpx_impl<double, int32_t>(-s_px32_max));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t>(u_px32_max + 1ULL));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t>(s_px32_max + 1LL));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t>(-s_px32_max - 1LL));
            static_assert(is_safe_conversion_glpx_impl<double, int32_t, float>(u_px32_max));
            static_assert(is_safe_conversion_glpx_impl<double, int32_t, float>(s_px32_max));
            static_assert(is_safe_conversion_glpx_impl<double, int32_t, float>(-s_px32_max));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t, float>(u_px32_max + 256ULL));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t, float>(s_px32_max + 256LL));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t, float>(-s_px32_max - 256LL));
            static_assert(is_safe_conversion_glpx_impl<double, int32_t, double>(u_px32_max));
            static_assert(is_safe_conversion_glpx_impl<double, int32_t, double>(s_px32_max));
            static_assert(is_safe_conversion_glpx_impl<double, int32_t, double>(-s_px32_max));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t, double>(u_px32_max + 1ULL));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t, double>(s_px32_max + 1LL));
            static_assert(!is_safe_conversion_glpx_impl<double, int32_t, double>(-s_px32_max - 1LL));
        }

        D_ASSERT(!errno);

        D_WARNING_POP;
    }
}

void test_shader_library() noexcept
{
    test_conversion_glpx();
}