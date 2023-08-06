#pragma once

#include <file/path.h>
#include <font/font.h>


namespace font_cache
{
    struct face_resource
    {
        using view_type = ::font::face_descriptor_t;

        view_type face;
        size_t cache_index;

        struct null_type
        {
            [[nodiscard]]
            constexpr operator face_resource () const noexcept
            {
                return
                {
                    .face{ nullptr },
                    .cache_index{ 0_uz }
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

    static_assert(std::is_same_v<null_t<face_resource>, face_resource::null_type>);
    static_assert(std::is_same_v<view_t<face_resource>, const face_resource::view_type>);

    struct cache_deref
    {
        void operator () (face_resource face) const noexcept;
    };

    using face = unique_resource<face_resource, cache_deref>;

    [[nodiscard]]
    face load_font(file::path_zstring_view name, px::pxside_t size) noexcept;
}
