#include <core/clamp_cast.h>

#include <core/assert.h>


#include <cerrno>
#include <cinttypes>

#include <utility>

namespace
{

    template<class Target, class Source>
    void test_clamp_cast_for() noexcept
    {
        constexpr Source s_min{ numeric_min_v<Source> };
        constexpr Source s_max{ numeric_max_v<Source> };
        constexpr Source s_one{ 1 };

        constexpr Target t_min{ numeric_min_v<Target> };
        constexpr Target t_zero{ 0 };
        constexpr Target t_one{ 1};
        constexpr Target t_max{ numeric_max_v<Target> };
        
        constexpr auto t_s_min = static_cast<Target>(s_min);
        constexpr auto t_s_max = static_cast<Target>(s_max);

        constexpr Target r_min
        { 
            (std::is_unsigned_v<Target> || std::is_unsigned_v<Source>) 
            ? t_zero 
            : ( (sizeof(Source) < sizeof(Target)) ? t_s_min : t_min ) 
        };

        constexpr Target r_max{ (numeric_digits_v<Source> < numeric_digits_v<Target>) ? t_s_max : t_max };

        static_assert(r_min >= t_min);
        static_assert(r_min < t_one);
        static_assert(r_max > t_one);
        static_assert(r_max <= t_max);

        static_assert(t_min == clamp_cast<Target>(t_min));
        static_assert(t_one == clamp_cast<Target>(t_one));
        static_assert(t_max == clamp_cast<Target>(t_max));

        static_assert(r_min == clamp_cast<Target>(s_min));
        static_assert(t_one == clamp_cast<Target>(s_one));
        static_assert(r_max == clamp_cast<Target>(s_max));

        D_ASSERT(!errno);
    }
}

void test_clamp_cast() noexcept
{
    {
        static_assert(hi_cast<uint16_t>(0xdeadbeeful) == 0xdeadu);
        static_assert(lo_cast<uint16_t>(0xdeadbeeful) == 0xbeefu);
        static_assert(hi_cast<uint32_t>(0xfeedfacedeadbeefull) == 0xfeedfaceul);
        static_assert(lo_cast<uint32_t>(0xfeedfacedeadbeefull) == 0xdeadbeeful);

        static_assert(hi_cast<uint8_t>(0xdeadbeeful) == 0xbeu);
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

    test_clamp_cast_for<uint8_t, uint8_t>();
    test_clamp_cast_for<uint8_t, uint16_t>();
    test_clamp_cast_for<uint8_t, uint32_t>();
    test_clamp_cast_for<uint8_t, uint64_t>();
    test_clamp_cast_for<uint8_t, int8_t>();
    test_clamp_cast_for<uint8_t, int16_t>();
    test_clamp_cast_for<uint8_t, int32_t>();
    test_clamp_cast_for<uint8_t, int64_t>();
    
    test_clamp_cast_for<uint16_t, uint8_t>();
    test_clamp_cast_for<uint16_t, uint16_t>();
    test_clamp_cast_for<uint16_t, uint32_t>();
    test_clamp_cast_for<uint16_t, uint64_t>();
    test_clamp_cast_for<uint16_t, int8_t>();
    test_clamp_cast_for<uint16_t, int16_t>();
    test_clamp_cast_for<uint16_t, int32_t>();
    test_clamp_cast_for<uint16_t, int64_t>();

    test_clamp_cast_for<uint32_t, uint8_t>();
    test_clamp_cast_for<uint32_t, uint16_t>();
    test_clamp_cast_for<uint32_t, uint32_t>();
    test_clamp_cast_for<uint32_t, uint64_t>();
    test_clamp_cast_for<uint32_t, int8_t>();
    test_clamp_cast_for<uint32_t, int16_t>();
    test_clamp_cast_for<uint32_t, int32_t>();
    test_clamp_cast_for<uint32_t, int64_t>();

    test_clamp_cast_for<uint64_t, uint8_t>();
    test_clamp_cast_for<uint64_t, uint16_t>();
    test_clamp_cast_for<uint64_t, uint32_t>();
    test_clamp_cast_for<uint64_t, uint64_t>();
    test_clamp_cast_for<uint64_t, int8_t>();
    test_clamp_cast_for<uint64_t, int16_t>();
    test_clamp_cast_for<uint64_t, int32_t>();
    test_clamp_cast_for<uint64_t, int64_t>();

    test_clamp_cast_for<int8_t, uint8_t>();
    test_clamp_cast_for<int8_t, uint16_t>();
    test_clamp_cast_for<int8_t, uint32_t>();
    test_clamp_cast_for<int8_t, uint64_t>();
    test_clamp_cast_for<int8_t, int8_t>();
    test_clamp_cast_for<int8_t, int16_t>();
    test_clamp_cast_for<int8_t, int32_t>();
    test_clamp_cast_for<int8_t, int64_t>();

    test_clamp_cast_for<int16_t, uint8_t>();
    test_clamp_cast_for<int16_t, uint16_t>();
    test_clamp_cast_for<int16_t, uint32_t>();
    test_clamp_cast_for<int16_t, uint64_t>();
    test_clamp_cast_for<int16_t, int8_t>();
    test_clamp_cast_for<int16_t, int16_t>();
    test_clamp_cast_for<int16_t, int32_t>();
    test_clamp_cast_for<int16_t, int64_t>();

    test_clamp_cast_for<int32_t, uint8_t>();
    test_clamp_cast_for<int32_t, uint16_t>();
    test_clamp_cast_for<int32_t, uint32_t>();
    test_clamp_cast_for<int32_t, uint64_t>();
    test_clamp_cast_for<int32_t, int8_t>();
    test_clamp_cast_for<int32_t, int16_t>();
    test_clamp_cast_for<int32_t, int32_t>();
    test_clamp_cast_for<int32_t, int64_t>();

    test_clamp_cast_for<int64_t, uint8_t>();
    test_clamp_cast_for<int64_t, uint16_t>();
    test_clamp_cast_for<int64_t, uint32_t>();
    test_clamp_cast_for<int64_t, uint64_t>();
    test_clamp_cast_for<int64_t, int8_t>();
    test_clamp_cast_for<int64_t, int16_t>();
    test_clamp_cast_for<int64_t, int32_t>();
    test_clamp_cast_for<int64_t, int64_t>();

    D_ASSERT(!errno);
}