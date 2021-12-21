#include <iostream>
#include <iomanip>
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
#include <array>
#include <cuchar>

#include <core/intrusive_list.h>
#include <core/small_vector.h>
#include <core/point2d.h>


template<class T>
void convert(std::basic_string_view<T> sp)
{
    D_ASSERT(sp.data());
}

int main() noexcept
{
    std::string s{"lolo"};
    convert(s);

    return 0;
}
