#include <bit>

#include <core/zero.h>

#include <core/limits.h>
#include <core/size_type.h>

void test_zero() noexcept
{
    static_assert(constexpr_abs(0) == 0);
    static_assert(constexpr_abs(0u) == 0u);
    static_assert(constexpr_abs(0_uz) == 0_uz);
    static_assert(constexpr_abs(0.0f) == 0.0f);

    static_assert(std::bit_cast<uint32_t>(0.0f) != std::bit_cast<uint32_t>(-0.0f));
    static_assert(std::bit_cast<uint32_t>(constexpr_abs(-0.0f)) == std::bit_cast<uint32_t>(-0.0f));
    static_assert(std::bit_cast<uint32_t>(constexpr_abs(0.0f)) == std::bit_cast<uint32_t>(0.0f));

    static_assert(std::bit_cast<uint64_t>(0.0) != std::bit_cast<uint64_t>(-0.0));
    static_assert(std::bit_cast<uint64_t>(constexpr_abs(-0.0)) == std::bit_cast<uint64_t>(-0.0));
    static_assert(std::bit_cast<uint64_t>(constexpr_abs(0.0)) == std::bit_cast<uint64_t>(0.0));

    static_assert(constexpr_abs(numeric_max_v<uint32_t>) == numeric_max_v<uint32_t>);
    static_assert(constexpr_abs(numeric_max_v<int32_t>) == numeric_max_v<int32_t>);
    static_assert(constexpr_abs(-numeric_max_v<int32_t>) == numeric_max_v<int32_t>);

    D_ASSERT(!errno);
}