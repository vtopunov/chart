#include <core/clamp_cast.h>

#include <core/assert.h>

#include <cerrno>
#include <cinttypes>

void test_clamp_cast() noexcept
{
    {
        static_assert(hi_cast<uint16_t>(0xdeadbeeful) == 0xdeadu);
        static_assert(lo_cast<uint16_t>(0xdeadbeeful) == 0xbeefu);
        static_assert(hi_cast<uint32_t>(0xfeedfacedeadbeefull) == 0xfeedfaceul);
        static_assert(lo_cast<uint32_t>(0xfeedfacedeadbeefull) == 0xdeadbeeful);

        static_assert(hi_cast<uint8_t>(0xdeadbeeful) == 0xdeu);
        static_assert(lo_cast<uint8_t>(0xdeadbeeful) == 0xefu);
        static_assert(hi_cast<uint32_t>(0xdeadbeeful) == 0ul);
        static_assert(lo_cast<uint32_t>(0xdeadbeeful) == 0xdeadbeeful);
    }

    {
        static_assert(clamp_cast<uint64_t>(0xfeedfacedeadbeefull) == 0xfeedfacedeadbeefull);
        static_assert(clamp_cast<uint32_t>(0xfeedfacedeadbeefull) == 0xfffffffful);
        static_assert(clamp_cast<uint32_t>(0xdeadbeefull) == 0xdeadbeeful);
        static_assert(clamp_cast<uint16_t>(0xfeedfacedeadbeefull) == 0xffffu);
        static_assert(clamp_cast<uint16_t>(0xbeefull) == 0xbeeful);
    }


    D_ASSERT(!errno);
}