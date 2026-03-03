
#include <execution>
#include <algorithm>
#include <bit>
#include <initializer_list>
#include <memory>
#include <optional>
#include <functional>

#include <core/fwd.h>
#include <core/unique_function.h>


namespace
{
    struct test_defecon
    {
        constexpr explicit test_defecon() noexcept = default;
    };
    constexpr test_defecon test_defecon_v{};

    void fn(test_defecon value) noexcept
    {}

    struct test_defecon2
    {
        constexpr explicit test_defecon2(test_defecon) noexcept
        {}

        D_DISABLE_COPYMOVE_CA(test_defecon2);
    };

    constexpr test_defecon2 get_test_defecon2() noexcept
    {
        return test_defecon2(test_defecon_v);
    }

    constexpr test_defecon get_test_defecon1() noexcept
    {
        return test_defecon{};
    }

    struct BA {};
    struct B : BA {};

    struct A
    {
        int value{ 1 };

        void operator () (B) const
        {
            D_ASSERT(!errno && value);
            D_ASSERT(!errno);
            D_ASSERT(!errno);
        }

        void operator () (BA)
        {
            D_ASSERT(!errno && value);
            D_ASSERT(!errno);
            D_ASSERT(!errno);
        }
    };
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) noexcept
{
    A lol;
    //const A clol;
    //std::move_only_function<void(B)> fn = lol; fn(B{}); // error // lol clol std::move(lol)
    //std::function<void(B)> fn2 = lol; fn2(B{}); // error // lol clol std::move(lol)
    ::function_view<void(B)> fn3 = lol; fn3(B{});
    ::unique_function<void(B)> fn4 = std::move(lol); fn3(B{});
    ::invoke_if_exist(A{}, B{});
    static_assert(is_invocable_v<A, B>);
    static_assert(!std::is_invocable_v<A, B>);
    static_assert(std::is_invocable_v<const A, B>);
    
    {
        A lol2;
        std::as_const(lol2)(B{});
    }

    {
        const A lol3;
        lol3(B{});
    }

    size_t args_len{};

    std::add_lvalue_reference_t<int&>;

    test_defecon2 value = get_test_defecon2();
    test_defecon2 value2 = test_defecon2(test_defecon2(test_defecon_v));

    std::unique_ptr<size_t> ptr{};

    const auto slrlen_acc = [&args_len] (const char* arg) noexcept 
    {
          args_len += strlen(arg);
    };

    std::is_function_v<decltype([] () {}) > ;

    std::optional<int> opt;
    opt.has_value();
    
    std::add_const_t<int&> x = *opt;

    std::for_each
    (
        std::execution::unseq,
        argv, argv + argc,
        slrlen_acc
    );

    std::is_convertible_v<void, void>;

    std::nullopt_t;

    return std::popcount(args_len);
}