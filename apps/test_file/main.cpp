extern void test_file_io() noexcept;
extern void test_file_mmap() noexcept;

int main() noexcept
{
    test_file_io();
    test_file_mmap();
}
