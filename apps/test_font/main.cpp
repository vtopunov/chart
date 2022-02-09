#include <font/font.cpp>

#include <file/file_mmap.h>

#include <process.h>

namespace
{
    constexpr const font::library_descriptor_t get_library() noexcept
    {
        return font::library::library_ref{};
    }

    void deref_library() noexcept
    {
        [[maybe_unused]]
        const font::library::library_ref library_deref
        {
            resource_construct,
            font::library::dtor_state::enabled
        };
    }

    void test_deref_and_destroy_library_cache() noexcept
    {
        D_ASSERT(nullptr != get_library());
        deref_library();
        D_ASSERT(nullptr == get_library());
    }
}

int main() noexcept
{
    const auto font_file = file::mmap(_PATH("..\\fonts\\DroidSerif-Regular.ttf"));
    if (!font_file)
    {
        e_debug("can't open font file");
        return {};
    }

    font::library_descriptor_t lib{ nullptr };

    {
        {
            const auto face = font::create_face(font_file, 20_px);
            if (!face)
            {
                e_debug("can't create font");
                return {};
            }

            lib = get_library();
            D_ASSERT(nullptr != lib);
        }

        D_ASSERT(get_library() == lib);
        test_deref_and_destroy_library_cache();
    }

    {
        {
            const auto face = font::create_face(font_file, 19_px);
            if (!face)
            {
                e_debug("can't create font");
                return {};
            }

            lib = get_library();
            D_ASSERT(nullptr != lib);

            deref_library();
            D_ASSERT(get_library() == lib);
        }

        D_ASSERT(nullptr == get_library());
    }

    {
        {
            const auto face18 = font::create_face(font_file, 18_px);
            const auto face17 = font::create_face(font_file, 17_px);
            if (!face18 || !face17)
            {
                e_debug("can't create font");
                return {};
            }

            lib = get_library();
            D_ASSERT(nullptr != lib);
        }

        D_ASSERT(get_library() == lib);
        test_deref_and_destroy_library_cache();
    }

    {
        {
            const auto face16 = font::create_face(font_file, 16_px);
            const auto face15 = font::create_face(font_file, 15_px);
            if (!face16 || !face15)
            {
                e_debug("can't create font");
                return {};
            }

            lib = get_library();
            D_ASSERT(nullptr != lib);

            deref_library();
            D_ASSERT(get_library() == lib);
        }

        D_ASSERT(nullptr == get_library());
    }

    {
        {
            auto face14 = font::create_face(font_file, 14_px);
            auto face13 = font::create_face(font_file, 13_px);
            if (!face14 || !face13)
            {
                e_debug("can't create font");
                return {};
            }

            lib = get_library();
            D_ASSERT(nullptr != lib);

            deref_library();
            D_ASSERT(get_library() == lib);

            deref_library();
            D_ASSERT(get_library() == lib);

            FT_Done_Face(face14.release());
            FT_Done_Face(face13.release());

            test_deref_and_destroy_library_cache();
        }

        D_ASSERT(nullptr == get_library());
    }

    {
        D_ASSERT(nullptr == get_library());
        [[maybe_unused]]
        static const auto static_last_face = font::create_face(font_file, 12_px);
        D_ASSERT(nullptr != get_library());
        _cexit();
        D_ASSERT(nullptr == get_library());
        exit(0);
    }

    return 0;
}
