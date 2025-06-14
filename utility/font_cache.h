#pragma once

#include <file/path.h>
#include <font/font.h>


namespace font_cache
{
    constexpr auto default_font_name = _PATH("OpenSans-Regular.ttf");
    constexpr auto default_font_size = 15_npx;

    struct cached_face_resource
    {
        using view_type = ::font::face_resource;

        view_type face;
        size_t cache_index;

        struct null_type
        {
            [[nodiscard]]
            constexpr operator cached_face_resource () const noexcept
            {
                return
                {
                    .face{ nullptr },
                    .cache_index{ 0u }
                };
            }
        };

        constexpr operator view_type () const noexcept
        {
            return face;
        }

        constexpr explicit operator bool() const noexcept
        {
            return !!face;
        }
    };

    static_assert(std::is_same_v<decl_null_type_t<cached_face_resource>, cached_face_resource::null_type>);
    static_assert(std::is_same_v<decl_view_type_t<cached_face_resource>, cached_face_resource::view_type>);

    struct cache_deref
    {
        static void unsafe_deref(cached_face_resource notnull_face) noexcept;

        constexpr void operator () (cached_face_resource face) const noexcept
        {
            if(face)
            {
                unsafe_deref(face);
            }
        }
    };

    using cached_face = unique_resource<cached_face_resource, cache_deref>;

    [[nodiscard]]
    cached_face clone(cached_face_resource r) noexcept;

    [[nodiscard]]
    cached_face load_font(file::path_zstring_view name, npx_t size) noexcept;

    [[nodiscard]]
    cached_face default_font() noexcept;
}