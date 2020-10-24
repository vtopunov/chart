#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <corecrt_io.h>
#include <stdio.h>
#include <bit>
#include <cmath>
#include <numbers>
#include <span>
#include <optional>
#include <fstream>
#include <sstream>
#include <charconv>
#include <vector>
#include <map>
#include <variant>

#include <core/underlying_cast.h>
#include <core/intrusive_list.h>
#include <core/small_vector.h>

namespace
{
    struct empty
    {};

    struct s_w_e
    {
        int64_t i;
        uint64_t j;
        [[no_unique_address]] empty e;

        int64_t gi() const
        {
            return i;
        }

        constexpr ~s_w_e()
        {
            i = 0;
        }
    };
   
    enum class i32_e : uint32_t
    {};

    struct i32
    {
        i32_e l;
    };

    struct i64
    {
        i32 l;
        uint32_t m;
    };

    constexpr bool always_true(bool b) noexcept
    {
        D_ASSERT(b);
        return b;
    }
}

int main() noexcept
{
    

    std::vector<s_w_e> v{{1, 2}, {3, 5}, {5, 6}};
    v.reserve(v.size() + 1);

    small_vector<s_w_e, 3> sv;
    sv.try_emplace_back(1, 2);
    sv.try_emplace(sv.begin(), 7, 8u);


    s_w_e swe{0, 0, {}};
    constexpr auto r = sizeof(swe);

    constexpr auto fls = always_true(true);
    
    constexpr auto rr = sizeof(i64);

    std::cout << rr;

    return 0;
}
