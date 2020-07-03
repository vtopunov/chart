
extern void test_narrow_cast() noexcept;
extern void test_underlying_cast() noexcept;
extern void test_span() noexcept;
extern void test_vec() noexcept;
extern void test_num_range() noexcept;
extern void test_point() noexcept;
extern void test_rect() noexcept;
extern void test_lerp() noexcept;
extern void test_rational() noexcept;
extern void test_color() noexcept;
extern void test_lerp_color() noexcept;
extern void test_handle() noexcept;
extern void test_small_vector() noexcept;
extern void test_small_flat_map() noexcept;
extern void test_buffer() noexcept;

int main() noexcept
{
    test_narrow_cast();
    test_underlying_cast();
    test_span();
    test_vec();
    test_num_range();
    test_point();
    test_rect();
    test_lerp();
    test_rational();
    test_color();
    test_lerp_color();
    test_handle();
    test_small_vector();
    test_small_flat_map();
    test_buffer();
}
