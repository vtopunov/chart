#include <file/file_mmap.h>


#include <random>
#include <span>

#include <core/assert.h>

#include <file/file_io.h>



void test_file_mmap() noexcept
{
    static_assert(std::is_trivial_v<file::file_mmap_resource> && std::is_standard_layout_v<file::file_mmap_resource>);

    using value_type = uint64_t;

    constexpr auto file_name = _PATH("test_mmap.txt");
    
    std::mt19937_64 content_generator{ std::random_device{}( ) };

    static_assert( std::is_same_v<decltype( content_generator() ), value_type> );

    value_type content[1024]{};
    constexpr auto content_size = sizeof(content);
    constexpr auto number_of_tests = 256_uz;
    
    for ( size_t i = 0; i < number_of_tests; ++i )
    {
        {
            for ( auto& value : content )
            {
                value = content_generator();
            }
            
            const auto w_size = write
            (
                file::wo_open(file_name, file::write_mode::rewrite), 
                content, 
                content_size
            );

            D_ASSERT(w_size == content_size);
        }

        {
            const auto mmap = file::mmap(file_name);
            D_ASSERT(mmap && mmap.r().size() == content_size);
           
            
            {
                const auto mmap_span = mmap.r().view().as_span<value_type>();
                D_ASSERT(mmap_span.front() == content[0]);
                D_ASSERT(mmap_span.size() == std::size(content));
            }
            
            D_ASSERT(!memcmp(mmap.r().data(), content, content_size));
        }
    }

    D_ASSERT(!errno);
}