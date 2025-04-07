#include <array>

#include "utility/font_cache.cpp"


#ifdef D_OS_WINDOWS
#define D_FONT_NAME(name) _PATH("..\\fonts\\" ## name)
#else
#define D_FONT_NAME(name) _PATH(name)
#endif


namespace
{
    constexpr std::array names
    {
        D_FONT_NAME("DroidSerif-Regular.ttf"),
        D_FONT_NAME("DroidSerif-Bold.ttf"),
        D_FONT_NAME("DroidSerif-Italic.ttf")
    };

    constexpr auto new_name = D_FONT_NAME("DroidSerif-BoldItalic.ttf");

    constexpr std::array npxs{ 15_npx, 10_npx };

    constexpr auto names_npxs_size = names.size() * npxs.size();
    static_assert(font_cache::faces_cache::storage_type::static_size == names_npxs_size);

    constexpr auto names_npxs = [] () noexcept
    {
        struct name_npx
        {
            std::decay_t<decltype(names[0])> name;
            std::decay_t<decltype(npxs[0])> px;
        };

        std::array<name_npx, names_npxs_size> result{};

        {
            size_t index = 0u;
            for (const auto npx : npxs)
            {
                for (const auto name : names)
                {
                    result[index] = { name, npx };
                    ++index;
                }
            }
        }

        return result;
    }();

    using const_faces_span = span<const font_cache::face, names_npxs_size>;
    using faces_span = span<font_cache::face, names_npxs_size>;
    using face_d_array = std::array<font::face_descriptor_t, names_npxs_size>;
    using const_face_d_span = span<const font::face_descriptor_t, names_npxs_size>;

    constexpr face_d_array make_face_d_array(const_faces_span fonts) noexcept
    {
        std::array<font::face_descriptor_t, names_npxs_size> views{};
        std::copy(fonts.begin(), fonts.end(), views.begin());
        return views;
    }

    constexpr bool is_unique(const_faces_span fonts) noexcept
    {
        auto views = make_face_d_array(fonts);
        std::sort(views.begin(), views.end());
        return std::adjacent_find(views.cbegin(), views.cend()) == views.cend();
    }

    void test_constexpr_instance() noexcept
    {
        constexpr font_cache::face face{};
        static_assert(!face);
        D_ASSERT(!errno);
    }

    void test_full_small_cache() noexcept
    {
        D_ASSERT(names_npxs_size == font_cache::global_faces_cache().size());
        D_ASSERT(names_npxs_size == font_cache::global_faces_cache().capacity());
    }

    void test_fill(faces_span fonts) noexcept
    {
        for (size_t i = 0; i < fonts.size(); ++i)
        {
            const auto [name, npx] = names_npxs[i];
            auto font = font_cache::load_font(name, npx);
            D_ASSERT(font);
            fonts[i] = std::move(font);
        }
        D_ASSERT(is_unique(fonts));
        test_full_small_cache();
    }

    void test_load_form_cache(faces_span fonts) noexcept
    {
        for (size_t i = 0; i < fonts.size(); ++i)
        {
            const auto [name, npx] = names_npxs[i];
            auto& font = fonts[i];

            const font::face_descriptor_t d_font{ font };

            {
                const auto identical_font = font_cache::load_font(name, npx);
                D_ASSERT(d_font == view(identical_font));
            }

            {
                font.reset();
                const auto unused_font = font_cache::load_font(name, npx + 3_npx);
                D_ASSERT(d_font == view(unused_font));
            }

            {
                font.reset();
                auto identical_unused_font = font_cache::load_font(name, npx);
                D_ASSERT(d_font == view(identical_unused_font));
                font = std::move(identical_unused_font);
            }
        }

        D_ASSERT(is_unique(fonts));
        test_full_small_cache();
    }

    void test_cached_faces(const_face_d_span faces_d) noexcept
    {
        test_full_small_cache();

        const auto& cache = font_cache::global_faces_cache();
        D_ASSERT(cache.size() == faces_d.size());

        for (size_t i = 0; i < faces_d.size(); ++i)
        {
            D_ASSERT(faces_d[i] == cache.value(i).face);
        }
    }

    void test_garbage_collection(faces_span fonts) noexcept
    {
        {
            auto views = make_face_d_array(fonts);

            for (auto& font : fonts)
            {
                font.reset();
            }

            test_cached_faces(views);

            constexpr auto max_npx = *std::max_element(npxs.begin(), npxs.end());
            static_assert((max_npx - fonts.size()) >= 8_npx);

            for (size_t i = 0; i < fonts.size(); ++i)
            {
                auto& font = fonts[i];
                D_ASSERT(!font);
                font = font_cache::load_font(new_name, narrow<npx_t>(max_npx - i));
                D_ASSERT(font);
                views[i] = font;
                test_cached_faces(views);
            }

            const auto new_views = make_face_d_array(fonts);
            D_ASSERT(new_views.size() == views.size());
            D_ASSERT(std::equal(new_views.cbegin(), new_views.cend(), views.cbegin()));
        }

        {
            auto views = make_face_d_array(fonts);

            for (size_t i = fonts.size(); i;)
            {
                fonts[--i].reset();
            }

            constexpr auto back_i = fonts.size() - 1_uz;
            for (size_t i = 0; i < fonts.size(); ++i)
            {
                const auto [name, px] = names_npxs[i];
                auto& font = fonts[i];
                D_ASSERT(!font);
                font = font_cache::load_font(name, px);
                D_ASSERT(font);
                views[back_i - i] = font;
                test_cached_faces(views);
            }
        }
    }
}


void test_font_cache() noexcept
{
    std::array<font_cache::face, names_npxs_size> fonts{};
    test_constexpr_instance();
    test_fill(fonts);
    test_load_form_cache(fonts);
    test_garbage_collection(fonts);
}