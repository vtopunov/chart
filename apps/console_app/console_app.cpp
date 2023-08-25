#include <iterator>
#include <optional>

#include <core/underlying.h>

struct event0 {};
struct event1 {};
struct event2 {};
struct event3 {};

enum class e_result_t : size_t
{
    invalid = std::numeric_limits<size_t>::max()
};

struct proc
{
    e_result_t operator () (const event0&) { return {}; }
    void operator () (const event1&) {}
    bool operator () (const event2&) { return true; }
};


template<class Fn, class Arg>
using return_t = std::remove_cv_t<
    decltype(std::declval<std::add_lvalue_reference_t<Fn>>()(std::declval<std::add_lvalue_reference_t<Arg>>()))
>;

template<class T, class E>
std::enable_if_t<std::is_same_v<return_t<T, E>, void>> call_event(T& function, const E& e) noexcept
{
    function(e);
}

template<class T, class E>
std::enable_if_t<std::is_same_v<return_t<T, E>, bool>> call_event(T& function, const E& e) noexcept
{
    function(e);
}

template<class T, class E>
std::enable_if_t<std::is_enum_v<return_t<T, E>>> call_event(T& function, const E& e) noexcept
{
    function(e);
}


struct no_overloaded
{
    template<class T>
    constexpr no_overloaded(const T&) noexcept
    {}
};

template<class T>
constexpr void call_event(T&, no_overloaded) noexcept
{}




int main()
{
    proc p;
    constexpr event0 e0;
    constexpr event1 e1;
    constexpr event2 e2;
    constexpr event3 e3;
    call_event(p, e0);
    call_event(p, e1);
    call_event(p, e2);
    call_event(p, e3);

    return 0;
}