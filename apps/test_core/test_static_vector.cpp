#include <string_view>

#include <core/static_vector.h>
#include <core/view.h>


namespace
{
    template<class T>
    constexpr bool is_eq_sv(std::string_view sv, const T& v) noexcept
    {
        const std::string_view v_sv{ std::data(v), std::size(v) };
        return v_sv == sv;
    }
}

void test_static_vector() noexcept
{
    constexpr auto size = 4_uz;
    static_vector<int, size> sv;
    D_ASSERT(0 == sv.size());
    D_ASSERT(size == sv.capacity());
    D_ASSERT(sv.try_emplace_back(0));
    D_ASSERT(sv.try_emplace_back(1));
    D_ASSERT(sv.try_emplace_back(2));
    D_ASSERT(sv.try_emplace_back(3));
    D_ASSERT(size == sv.size());
    D_ASSERT(size == sv.capacity());
    D_ASSERT(!sv.try_emplace_back(4));
    D_ASSERT(!sv.try_emplace_back(5));
    D_ASSERT(!sv.try_emplace_back(6));
    D_ASSERT(size == sv.size());
    D_ASSERT(size == sv.capacity());
    D_ASSERT(sv.try_reserve(4));
    D_ASSERT(!sv.try_reserve(5));
    D_ASSERT(size == sv.size());
    D_ASSERT(size == sv.capacity());

    {
        static constexpr auto msg_abc = [] () noexcept
        {
            constexpr auto n_abc = u_next('Z') - 'A';
            constexpr auto n_abc_small = u_next('z') - 'a';
            static_assert(n_abc == n_abc_small);

            std::array<char, 2u * n_abc> test{};

            for (char i = '\0'; i < n_abc; ++i)
            {
                test[i] = 'A' + i;
                test[i + n_abc] = 'a' + i;
            }

            return test;
        } ();

        {
            constexpr std::string_view bad_alloc_msg_sv{ small_vector_exceptions::bad_alloc_msg };

            {
                constexpr auto static_size = msg_abc.size() / 2;
                static_assert(bad_alloc_msg_sv.size() < static_size);
                static_vector<char, static_size> bad_alloc_test(
                    msg_abc,
                    small_vector_exceptions::accept_and_write_bad_alloc
                );

                D_ASSERT(is_eq_sv(bad_alloc_msg_sv, bad_alloc_test));
            }

            {
                constexpr auto static_size = bad_alloc_msg_sv.size() / 2;
                static_assert(static_size < bad_alloc_msg_sv.size());
                constexpr auto bad_alloc_half_msg_sv = bad_alloc_msg_sv.substr(0u, static_size);

                static_vector<char, static_size> bad_alloc_test(
                    msg_abc,
                    small_vector_exceptions::accept_and_write_bad_alloc
                );

                D_ASSERT(!is_eq_sv(bad_alloc_msg_sv, bad_alloc_test));
                D_ASSERT(is_eq_sv(bad_alloc_half_msg_sv, bad_alloc_test));
            }
        }

        {
            constexpr auto write_small_abc = [abc = view(msg_abc)] (char* data, size_t data_capacity) noexcept
            {
                size_t n_write = 0;

                if (data_capacity)
                {
                    for (const auto ch : abc)
                    {
                        if (ch >= 'a' && ch <= 'z')
                        {
                            data[n_write] = ch;
                            ++n_write;

                            if (data_capacity == n_write)
                            {
                                break;
                            }
                        }
                    }
                }

                return n_write;
            };

            static_vector<char, msg_abc.size()> only_small_abc
            {
                memory_overwrite_construct,
                msg_abc.size(),
                write_small_abc,
                small_vector_exceptions::accept_bad_alloc
            };

            D_ASSERT(msg_abc.size() / 2 == only_small_abc.size());
        }
    }
}