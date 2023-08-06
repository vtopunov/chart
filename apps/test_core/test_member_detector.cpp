#include <core/member_detector.h>
#include <core/assert.h>

#include <cstdint>
#include <cerrno>

namespace
{
    template<class T>
    using copy_assign_t = decltype(std::declval<T&>() = std::declval<const T&>());

    struct with_cp {};
    struct without_cp { void operator=(const without_cp&) = delete; };

    struct with_void_cp 
    {
        constexpr void operator=(const with_void_cp&) noexcept
        {}
    };


    template<class Ex>
    struct with_ex_cp
    {
        constexpr Ex operator=(const with_ex_cp&) noexcept
        {
            return {};
        }
    };

    template<class It>
    using decl_difference_t = typename It::difference_type;

    template<class It>
    using difference_t = detected_or_t<std::ptrdiff_t, decl_difference_t, It>;

    template<class DT>
    struct with_decl_difference { using difference_type = DT; };
    struct without_decl_difference {};
}

void test_member_detector() noexcept
{
    static_assert(is_detected_v<copy_assign_t, with_cp>);
    static_assert(!is_detected_v<copy_assign_t, without_cp>);

    static_assert(is_detected_exact_v<with_cp&, copy_assign_t, with_cp>);
    static_assert(is_detected_exact_v<void, copy_assign_t, with_void_cp>);
    static_assert(is_detected_exact_v<with_cp, copy_assign_t, with_ex_cp<with_cp> >);
    static_assert(is_detected_exact_v<with_cp*, copy_assign_t, with_ex_cp<with_cp*> >);

    static_assert(std::is_same_v<int16_t, difference_t<with_decl_difference<int16_t>>>);
    static_assert(std::is_same_v<int32_t, difference_t<with_decl_difference<int32_t>>>);
    static_assert(std::is_same_v<int64_t, difference_t<with_decl_difference<int64_t>>>);
    static_assert(std::is_same_v<ptrdiff_t, difference_t<with_decl_difference<ptrdiff_t>>>);
    static_assert(std::is_same_v<ptrdiff_t, difference_t<without_decl_difference>>);

    D_ASSERT(!errno);
}