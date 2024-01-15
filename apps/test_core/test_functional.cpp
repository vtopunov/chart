#include <core/functional.h>


void test_functional() noexcept
{
    constexpr optional_reference_wrapper<int> orw{};
    static_assert(!orw);
    static_assert(!orw.has_value());
    
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<int>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const int>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<int&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const int&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<int&&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const int&&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<optional_reference_wrapper<int>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<optional_reference_wrapper<const int>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const optional_reference_wrapper<const int>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<optional_reference_wrapper<int&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int&>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<optional_reference_wrapper<const int&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int&>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const optional_reference_wrapper<const int&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<optional_reference_wrapper<int&&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int&&>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<optional_reference_wrapper<const int&&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int&&>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const optional_reference_wrapper<const int&&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<optional_reference_wrapper<int>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<optional_reference_wrapper<const int>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const optional_reference_wrapper<const int>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<optional_reference_wrapper<int&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int&>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<optional_reference_wrapper<const int&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int&>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const optional_reference_wrapper<const int&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<optional_reference_wrapper<int&&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int&&>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<optional_reference_wrapper<const int&&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const optional_reference_wrapper<int&&>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const optional_reference_wrapper<const int&&>&>>);

    static_assert(std::is_same_v<int, remove_reference_wrapper_t<std::reference_wrapper<int>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<std::reference_wrapper<const int>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const std::reference_wrapper<const int>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<std::reference_wrapper<int&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int&>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<std::reference_wrapper<const int&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int&>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const std::reference_wrapper<const int&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<std::reference_wrapper<int&&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int&&>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<std::reference_wrapper<const int&&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int&&>>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const std::reference_wrapper<const int&&>>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<std::reference_wrapper<int>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<std::reference_wrapper<const int>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const std::reference_wrapper<const int>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<std::reference_wrapper<int&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int&>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<std::reference_wrapper<const int&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int&>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const std::reference_wrapper<const int&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<std::reference_wrapper<int&&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int&&>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<std::reference_wrapper<const int&&>&>>);
    static_assert(std::is_same_v<int, remove_reference_wrapper_t<const std::reference_wrapper<int&&>&>>);
    static_assert(std::is_same_v<const int, remove_reference_wrapper_t<const std::reference_wrapper<const int&&>&>>);

    D_ASSERT(!errno);
}