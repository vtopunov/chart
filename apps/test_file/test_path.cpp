#include <file/path.h>

#include <core/utility.h>


void test_path() noexcept
{
    static_assert(std::is_same_v<file::path_char_t, std::filesystem::path::value_type>);

    using path_char_t = string_char_t<decltype(_PATH("test0"))>;
    static_assert(std::is_same_v<path_char_t, file::path_char_t>);

    D_ASSERT(!errno);
}