
extern void test_type_traits() noexcept;
extern void test_value_type() noexcept;
extern void test_zero() noexcept;
extern void test_utility() noexcept;
extern void test_clamp_cast() noexcept;
extern void test_size_type() noexcept;
extern void test_narrow() noexcept;
extern void test_buffer_view() noexcept;
extern void test_zstring_view() noexcept;
extern void test_vec2() noexcept;
extern void test_point2d() noexcept;
extern void test_size2d() noexcept;
extern void test_num_range() noexcept;
extern void test_rect() noexcept;
extern void test_lerp() noexcept;
extern void test_rational() noexcept;
extern void test_color() noexcept;
extern void test_lerp_color() noexcept;
extern void test_null() noexcept;
extern void test_resource() noexcept;
extern void test_small_vector() noexcept;
extern void test_buffer() noexcept;
extern void test_utf() noexcept;

int main() noexcept
{
    test_type_traits();
    test_value_type();
    test_zero();
    test_utility();
    test_clamp_cast();
    test_size_type();
    test_narrow();
    test_buffer_view();
    test_zstring_view();
    test_vec2();
    test_point2d();
    test_size2d();
    test_num_range();
    test_rect();
    test_lerp();
    test_rational();
    test_color();
    test_lerp_color();
    test_null();
    test_resource();    
    test_buffer();
    test_small_vector();
    test_utf();

    return 0;
}
