extern void test_path() noexcept;
extern void test_file_io() noexcept;
extern void test_file_mmap() noexcept;


int main() noexcept
{
    test_path();
    test_file_io();
    test_file_mmap();
}
