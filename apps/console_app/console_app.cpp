#include <iostream>
#include <ranges>
#include <vector>
#include <string_view>

int main()
{
    uint32_t width = -7;
    uint32_t h = -9;
    int32_t signed_offset = h - width;
    return signed_offset;
}