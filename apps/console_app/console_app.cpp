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
#include <array>

#include <core/underlying_cast.h>
#include <core/intrusive_list.h>
#include <core/small_vector.h>
#include <core/point2d.h>

using namespace std::string_view_literals;

template<class T>
struct index
{
    using type = T;

    operator size_t () const noexcept
    {
        const auto size = sizeof(T);
        return size;
    }
};


template <class... As>
struct vi
{};

template<class Vertex, size_t... I>
struct vi<Vertex, std::index_sequence<I...>>
{

};

template <class... As>
struct vs
{

    inline static size_t as[]
    {
        index<As>{}...
    };
};


struct swp
{
    void swap(swp& o) noexcept
    {
        D_ASSERT(false);
    }
};

int main() noexcept
{    
    const auto& vas = vs<int, size_t, char>::as;

    swp swp0, swp1;

    std::swap(swp0, swp1);
    
    constexpr point2d_px_t p = vec2_cast<point2d_px_t>(vec2px_t{1, 2});

 
    const auto v = vec2_cast<std::vector<int>>(vec2px_t{ 1, 2 });
 

    constexpr std::array<int, 5u> arr{-1};

    return 0;
}
