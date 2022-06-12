#include <file/file_io.h>



int main() noexcept
{
    constexpr char hw[] = "hello world!";
    constexpr auto len = sizeof(hw) - 1u;
    file::write(file::stdout_fd, hw, len);
    file::write(file::stdout_fd, hw, len);
    file::write(file::stdout_fd, hw, len);
    return 0;
}
