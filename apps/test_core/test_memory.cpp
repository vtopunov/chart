#include <core/memory.h>


void test_memory() noexcept
{
    static constexpr char ma[8][8]{};
    constexpr auto ma_line_size = std::size(ma[0]);
    constexpr span<const char> p_first_ma{ std::data(ma[0]), sizeof(ma) - ma_line_size };

    for (const auto& line : ma)
    {
        const auto cbegin_line = std::cbegin(line);
        const auto cend_line = std::cend(line);
        D_ASSERT(ma_line_size == u_distance(cbegin_line, cend_line));

        for (const auto& value : p_first_ma)
        {
            const auto p_first = std::addressof(value);
            const auto p_last = p_first + ma_line_size;
            D_ASSERT(p_last <= (std::data(ma[0]) + sizeof(ma)));

            const auto in_newmem_left = (p_last <= cbegin_line);
            const auto in_newmem_right = (p_first >= cend_line);
            const auto in_newmem = in_newmem_left || in_newmem_right;
            
            D_ASSERT(in_newmem == is_newmem(p_first, cbegin_line, cend_line));
            D_ASSERT(in_newmem == is_newmem(p_first, cbegin_line, ma_line_size));
        }
    }


}