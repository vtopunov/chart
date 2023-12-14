
extern void test_event_matching() noexcept;
extern void test_event_processors_storage() noexcept;
extern void test_manipulator() noexcept;


int main() noexcept
{
    test_event_matching();
    test_event_processors_storage();
    test_manipulator();
    return 0;
}
