#include <core/tuple.h>

#include <core/small_vector.h>


void test_tuple() noexcept
{
    {
        static constexpr tuple<int, char, double, void*, unsigned> t{ 1, '\x2', 3, nullptr, 5u };
        constexpr auto p4t = get_ptr<4>(&t);
        static_assert(std::is_same_v<const unsigned* const, decltype(p4t)>);
        static_assert(p4t == std::addressof(t._4));
    }

    {
        const tuple<int, char, double, void*, unsigned, const char*> t{ 1, '\x2', 3, nullptr, 5u, "test_tuple" };
        const auto p4t = get_ptr<4>(&t);
        static_assert(std::is_same_v<const unsigned* const, decltype(p4t)>);
        D_ASSERT(5u == *p4t);
        const auto p5t = get_ptr<5>(&t);
        D_ASSERT('t' == (*p5t)[0]);
        D_ASSERT('e' == (*p5t)[1]);
    }

    {
        struct function_header
        {
            tuple<void (*) (void*), void (*) (void*, void*), void (*) (int)> fn_tuple;
            char data[nbyte_arch];
        } hdr;

        struct my_function
        {
            char data[300];
        } fn;

        {
            char ch{ '0' };
            for (auto& fnch : fn.data)
            {
                fnch = ch;

                if (ch < '9')
                {
                    ++ch;
                }
                else
                {
                    ch = '0';
                }
            }
        }

        constexpr auto hdr_size = sizeof(function_header);
        constexpr auto static_data_size = sizeof(hdr.data);
        constexpr auto my_function_size = sizeof(my_function);
        static_assert(my_function_size > static_data_size);
        constexpr auto my_function_oversize = my_function_size - static_data_size;
        constexpr auto floor_div_my = my_function_oversize / hdr_size;
        constexpr auto ceil_div_my = ceil_div(my_function_oversize, hdr_size);
        constexpr auto floor_size = floor_div_my * hdr_size;
        constexpr auto ceil_size = ceil_div_my * hdr_size;
        static_assert(floor_size < sizeof(my_function));
        static_assert(ceil_size > sizeof(my_function));

        small_vector<function_header> data{};
        data.reserve(ceil_size + 1u);
        data.emplace_back();
        memcpy(std::data(data.front().data), &fn, sizeof(fn));
        D_ASSERT(data.size());
    }
}