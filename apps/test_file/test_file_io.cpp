#include <span>

#include <core/size_type.h>
#include <core/narrow.h>

#include <file/file_io.h>

namespace
{
    constexpr auto file_name = _PATH("test_io.txt");
    constexpr std::string_view test_data{ "0123456789" };

    constexpr auto block_size = test_data.size() * sizeof(*test_data.data());
    constexpr auto block_offset = narrow_cast<file::offset_t>( block_size );

    constexpr size_t size_blocks(size_t n_blocks) noexcept
    {
        return block_size * n_blocks;
    }

    constexpr void check_block_size(size_t size) noexcept
    {
        D_ASSERT(size == block_size);
    }

    void check_equ(std::span<const char> left, std::span<const char> rigth) noexcept
    {
        D_ASSERT(left.size() == rigth.size());
        D_ASSERT(!memcmp(left.data(), rigth.data(), rigth.size()));
    }

    bool try_remove() noexcept
    {
        std::error_code errc{};
        const auto result = std::filesystem::remove(file_name, errc);
        return result && !errc;
    }

    void test_write(size_t n_blocks, file::write_mode mode) noexcept
    {
        const auto wof = file::wo_open(file_name, mode);
        D_ASSERT(wof);

        for ( size_t i = 0_uz; i < n_blocks; ++i )
        {
            check_block_size(write(wof, test_data.data(), block_size));
        }
    }

    void test_read(size_t n_blocks) noexcept
    {
        const auto rof = file::ro_open(file_name);
        D_ASSERT(rof);

        for ( size_t i = 0_uz; i < n_blocks; ++i )
        {
            char buffer[block_size]{};
            check_block_size(read(rof, buffer, block_size));
            check_equ(buffer, test_data);
        }

        {
            char ch[1]{};
            D_ASSERT(!read(rof, ch, sizeof(ch)));
            D_ASSERT(size(rof) == size_blocks(n_blocks));
        }
    }

    void test_write_mode() noexcept
    {
        test_write(3_uz, file::write_mode::truncate);
        test_read(3_uz);
        test_write(4_uz, file::write_mode::rewrite);
        test_read(4_uz);
        test_write(2_uz, file::write_mode::append);
        test_read(6_uz);
        test_write(5_uz, file::write_mode::rewrite);
        test_read(6_uz);
        test_write(0_uz, file::write_mode::truncate);
        test_read(0_uz);

        D_ASSERT(!file::wo_open(file_name, file::write_mode::create));
        D_ASSERT(try_remove());
        D_ASSERT(!file::ro_open(file_name));
        
        test_write(5_uz, file::write_mode::create);
        test_read(5_uz);
    }

    void test_rw() noexcept
    {
        constexpr auto n_blocks = 7_uz;
        constexpr auto file_size = size_blocks(n_blocks);
        constexpr auto end_offset = narrow_cast<file::offset_t>( file_size );

        test_write(n_blocks, file::write_mode::truncate);
        test_read(n_blocks);

        const auto rwf = file::rw_open(file_name, file::write_mode::rewrite);
        D_ASSERT(rwf);

        auto test_read_block = [&rwf] (std::span<const char> test) noexcept
        {
            char buffer[block_size]{};
            check_block_size(read(rwf, buffer, block_size));
            check_equ(buffer, test);
        };

        test_read_block(test_data);

        char w_test[block_size]{};
        std::reverse_copy(std::cbegin(test_data), std::cend(test_data), w_test);

        auto write_test_block = [&w_test, &rwf] () noexcept
        {
            check_block_size(write(rwf, w_test, block_size));
        };

        write_test_block();
        test_read_block(test_data);

        D_ASSERT(seek(rwf, -3 * block_offset, file::seek_mode::current) == 0);
        test_read_block(test_data);
        test_read_block(w_test);
        test_read_block(test_data);
        write_test_block();

        D_ASSERT(seek(rwf, block_offset, file::seek_mode::begin) == block_offset);
        test_read_block(w_test);
        test_read_block(test_data);
        test_read_block(w_test);
        test_read_block(test_data);
        write_test_block();

        D_ASSERT(seek(rwf, -end_offset, file::seek_mode::end) == 0);
        test_read_block(test_data);
        test_read_block(w_test);
        test_read_block(test_data);
        test_read_block(w_test);
        test_read_block(test_data);
        test_read_block(w_test);
        test_read_block(test_data);

        {
            char ch[1]{};
            D_ASSERT(!read(rwf, ch, sizeof(ch)));
            D_ASSERT(size(rwf) == size_blocks(n_blocks));
        }
    }

    void test_size(size_t n_blocks) noexcept
    {
        const auto test_size = size_blocks(n_blocks);
        D_ASSERT(size(file::ro_open(file_name)) == test_size);
        D_ASSERT(size(file::wo_open(file_name, file::write_mode::rewrite)) == test_size);
        D_ASSERT(size(file::wo_open(file_name, file::write_mode::append)) == test_size);
        D_ASSERT(size(file::rw_open(file_name, file::write_mode::rewrite)) == test_size);
        D_ASSERT(size(file::rw_open(file_name, file::write_mode::append)) == test_size);
    }

    void test_size() noexcept
    {
        constexpr auto start_blocks = 10_uz;
        test_write(start_blocks, file::write_mode::truncate);
        test_read(start_blocks);
        test_size(start_blocks);

        for ( size_t blocks = start_blocks; blocks <= 20; )
        {
            test_write(1_uz, file::write_mode::append);
            ++blocks;
            test_read(blocks);
            test_size(blocks);
        }
    }
}

void test_file_io() noexcept
{
    static_assert(!std::is_same_v<std::remove_cv_t<file::file_descriptor>, void*>);
    static_assert(std::is_trivial_v<file::file_descriptor> && std::is_standard_layout_v<file::file_descriptor>);
    static_assert(std::is_trivial_v<file::file_resource> && std::is_standard_layout_v<file::file_resource>);
    static_assert(std::is_trivial_v<file::ro_file_resource> && std::is_standard_layout_v<file::ro_file_resource>);
    static_assert(std::is_trivial_v<file::rw_file_resource> && std::is_standard_layout_v<file::rw_file_resource>);

    test_write_mode();
    test_size();
    test_rw();
    D_ASSERT(!errno);
}