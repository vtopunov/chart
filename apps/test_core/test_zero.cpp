#include <core/zero.h>

#include <cerrno>
#include <cstdint>
#include <cstring>

#include <core/size_type.h>
#include <core/rational.h>


namespace
{
    template<class To, class From>
    constexpr To bit_cast(const From& from) noexcept
    {
        To to;
        std::memcpy(&to, &from, sizeof(to));
        return to;
    }
}

void test_zero() noexcept
{
    static_assert(constexpr_abs(0) == 0);
    static_assert(constexpr_abs(0u) == 0u);
    static_assert(constexpr_abs(0_uz) == 0_uz);
    static_assert(constexpr_abs(0.0f) == 0.0f);

    D_ASSERT(bit_cast<uint32_t>(0.0f) != bit_cast<uint32_t>(-0.0f));
    D_ASSERT(bit_cast<uint32_t>(constexpr_abs(-0.0f)) == bit_cast<uint32_t>(-0.0f));
    D_ASSERT(bit_cast<uint32_t>(constexpr_abs(0.0f)) == bit_cast<uint32_t>(0.0f));

    D_ASSERT(bit_cast<uint64_t>(0.0) != bit_cast<uint64_t>(-0.0));
    D_ASSERT(bit_cast<uint64_t>(constexpr_abs(-0.0)) == bit_cast<uint64_t>(-0.0));
    D_ASSERT(bit_cast<uint64_t>(constexpr_abs(0.0)) == bit_cast<uint64_t>(0.0));

    static_assert(constexpr_abs(numeric_max_v<uint32_t>) == numeric_max_v<uint32_t>);
    static_assert(constexpr_abs(numeric_max_v<int32_t>) == numeric_max_v<int32_t>);
    static_assert(constexpr_abs(-numeric_max_v<int32_t>) == numeric_max_v<int32_t>);

    static_assert(0 == zero_v<>);
    static_assert(0.0 == zero_v<>);

    D_ASSERT(!errno);
}