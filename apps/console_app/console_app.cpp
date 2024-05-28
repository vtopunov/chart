#include <iostream>
#include <vector>
#include <array>
#include <tuple>


namespace
{
    struct Empty {}; // empty class

    struct X
    {
        int i;
        Empty e;
    };

    struct Y
    {
        int i;
        [[msvc::no_unique_address]] Empty e;
    };

    struct preY
    {
        [[msvc::no_unique_address]] Empty e;
        int i;
    };


    struct Z
    {
        char c;
        [[msvc::no_unique_address]] Empty e1, e2;
    };

    struct W
    {
        char c[2];
        [[msvc::no_unique_address]] Empty e1, e2;
    };

    void print(auto rem, const std::vector<int>& c)
    {
        for (std::cout << rem; const int el : c)
            std::cout << el << ' ';
        std::cout << '\n';
    }

    int ret_i_f() noexcept
    {
        return {};
    }

    float ret_f_f() noexcept
    {
        return {};
    }

    void ret_v_f() noexcept
    {}
}

int main() noexcept
{
    static_assert(sizeof(Empty) >= 1);

    static_assert(sizeof(X) >= sizeof(int) + 1);

    std::cout << "sizeof(Y) == sizeof(int) is " << std::boolalpha
        << (sizeof(Y) == sizeof(int)) << '\n';

    std::cout << "sizeof(preY) == sizeof(int) is " << std::boolalpha
        << (sizeof(preY) == sizeof(int)) << '\n';


    static_assert(sizeof(Z) >= 2);

    std::cout << "sizeof(W) == 2 is " << (sizeof(W) == 2) << '\n';

    {
        std::string long_string("Where is the end?");
        std::string short_string("Hi");

        std::cout
            << "String: before \"" << long_string << "\", ";
        long_string.resize(long_string.size() - 5u);
        std::cout << "after  \"" << long_string << '\n';

        std::cout
            << "String: before \"" << short_string << "\", ";
        short_string.resize(short_string.size() - 1u);
        std::cout << "after  \"" << short_string << "\"\n";
    }

    {
        std::vector<int> c = { 1, 2, 3 };
        print("The vector holds: ", c);

        c.resize(5);
        print("After resize up to 5: ", c);

        c.resize(2);
        print("After resize down to 2: ", c);

        c.resize(6, 4);
        print("After resize up to 6 (initializer = 4): ", c);
    }

    {
        std::array<int, 0u> arri_0{};
        std::cout << "array 0" << std::distance(arri_0.cbegin(), arri_0.cend()) << '\n';

        std::array arrai_1{ 0, 1, 3 };

        size_t n = 0;
        bool has{ true };
        n += has;
        std::cout << n << '\n';
    }

    {
        std::tuple<bool> zx;
        std::tuple<> x, y, z;
        const auto t = std::tuple_cat(x, y, z, zx);

    }

    {
        int arr[] = { 1, 2, 3, 4, 5 };

        auto sizeof_1 = [arr] {
            return sizeof(arr);
        };

        auto sizeof_2 = [arr = arr] {
            return sizeof(arr);
        };

        auto sizeof_3 = [=] {
            return sizeof(arr);
        };

        std::cout << "sz a 1: " << sizeof_1() << std::endl;
        std::cout << "sz a 2: " << sizeof_2() << std::endl;
        std::cout << "sz a 3: " << sizeof_3() << std::endl;
    }

    using result_t = std::make_unsigned_t<unsigned>;
    result_t r{};
    return r;
}