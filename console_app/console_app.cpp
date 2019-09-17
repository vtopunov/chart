
#include <crtdbg.h>
#include <new>
#include <utility>
#include <string>
#include <vector>
#include <queue>
#include <iostream>


int main()
{

    auto foo = []<int x>(std::integral_constant<int, x> )
    {
        constexpr auto xx = x;

        std::cout << xx;
    };

    foo( std::integral_constant<int, 0>() );
}

