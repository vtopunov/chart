#include "font_cache.h"

#include <filesystem>
#include <variant>

#include <utility/cache_storage.h>

#include <debug/debug.h>

#include <file/file_asset.h>
#include <file/file_mmap.h>


namespace font_cache
{
    namespace
    {
        struct asset_or_file_mmap
        {
            using mmap_variant_type = std::variant<std::monostate, file::asset_mmap, file::file_mmap>;

            mmap_variant_type mmap_variant;

            [[nodiscard]]
            constexpr explicit operator bool() const noexcept
            {
                return !!mmap_variant.index();
            }

            [[nodiscard]]
            constexpr operator const_buffer_view() const noexcept
            {
                const auto p = std::addressof(mmap_variant);

                if (const auto p_asset = std::get_if<file::asset_mmap>(p)) [[likely]]
                {
                    return view(*p_asset);
                }

                    if (const auto p_file = std::get_if<file::file_mmap>(p))
                    {
                        return view(*p_file);
                    }

                return {};
            }
        };

        [[nodiscard]]
        asset_or_file_mmap mmap_asset_or_file(file::path_zstring_view name) noexcept
        {
            if (auto asset_mmap = file::asset::mmap(name)) [[likely]]
            {
                return { .mmap_variant{ std::move(asset_mmap) } };
            }

                if (auto file_mmap = file::mmap(name))
                {
                    return { .mmap_variant{ std::move(file_mmap) } };
                }

            return {};
        }

        struct mmap_item
        {
            file::path_string name;
            asset_or_file_mmap mmap;

            struct by_name
            {
                file::path_string_view name;

                [[nodiscard]]
                constexpr bool operator () (const mmap_item& item) const noexcept
                {
                    return item.name == name;
                }
            };
        };

        struct face_item
        {
            font::face face;
            size_t mmap_id;
            npx_t size;

            struct by_face
            {
                font::face_descriptor_t face;

                [[nodiscard]]
                constexpr bool operator () (const face_item& item) const noexcept
                {
                    return face == item.face;
                }
            };

            struct by_mmap
            {
                size_t id;

                [[nodiscard]]
                constexpr bool operator () (const face_item& item) const noexcept
                {
                    return id == item.mmap_id;
                }
            };

            struct by_size
            {
                npx_t size;

                [[nodiscard]]
                constexpr bool operator () (const face_item& item) const noexcept
                {
                    return size == item.size;
                }
            };
        };

        using mmaps_cache = cache_storage<mmap_item, 6_uz>;
        using faces_cache = cache_storage<face_item, 6_uz>;
        using faces_pointer = typename faces_cache::pointer;

        [[nodiscard]]
        mmaps_cache& global_mmaps_cache() noexcept
        {
            static mmaps_cache cache{};
            return cache;
        }

        [[nodiscard]]
        faces_cache& global_faces_cache() noexcept
        {
            static faces_cache cache{};
            return cache;
        }
    }

    void cache_deref::unsafe_deref(face_resource notnull_face) noexcept
    {
        D_ASSERT(notnull_face);

        auto& faces = global_faces_cache();
        auto& mmaps = global_mmaps_cache();

        auto& item = faces.at(notnull_face.cache_index);
        auto& mmap_item = mmaps.at(item.mmap_id);

        item.deref(faces);
        mmap_item.deref(mmaps);
    }

    face clone(const face_resource face_r) noexcept
    {
        if (face_r)
        {
            auto& mmaps = global_mmaps_cache();
            auto& faces = global_faces_cache();

            auto& item = faces.at(face_r.cache_index);
            auto& mmap_item = mmaps.at(item.mmap_id);

            mmap_item.ref();
            item.ref();
        }

        return face
        {
            face_r.face,
            face_r.cache_index
        };
    }

    face load_font(file::path_zstring_view name, const npx_t size) noexcept
    {
        const file::path_string_view name_sv{ name.c_str() };
        auto& mmaps = global_mmaps_cache();
        auto& faces = global_faces_cache();

        auto cached_mmap = mmaps.select(mmap_item::by_name{ name_sv });

        faces_pointer cached_face{ nullptr };
        if (cached_mmap)
        {
            const auto [identical, garbage] = faces.select_with_garbage
            (
                face_item::by_mmap{ mmaps.index(cached_mmap) },
                face_item::by_size{ size }
            );

            if (identical)
            {
                cached_face = identical;
            }
            else if (garbage)
            {
                if (!font::size(garbage->face, size)) [[unlikely]]
                {
                    return {};
                }
                garbage->size = size;
                cached_face = garbage;
            }
        }

        if (!cached_face)
        {
            const_buffer_view font_storage;
            asset_or_file_mmap file_mmap;

            if (cached_mmap)
            {
                font_storage = view(cached_mmap->mmap);
            }
            else
            {
                file_mmap = mmap_asset_or_file(name);
                if (!file_mmap) [[unlikely]]
                {
                    e_debug(_PATH("can't open font file: {}"), name_sv);
                    return {};
                }

                font_storage = view(file_mmap);
            }

            auto face = font::create_face(font_storage, size);
            if (!face) [[unlikely]]
            {
                e_debug
                (
                    _PATH("create font face error: font file = {}, font size = {}"),
                    name_sv,
                    size
                );
                return {};
            }

                if (!cached_mmap)
                {
                    cached_mmap = mmaps.try_emplace(file::path_string(name_sv), std::move(file_mmap));
                    if (!cached_mmap) [[unlikely]]
                    {
                        e_debug("load_font: out of memory");
                        return {};
                    }
                }

            cached_face = faces.try_emplace(std::move(face), mmaps.index(cached_mmap), size);
            if (!cached_face) [[unlikely]]
            {
                e_debug("load_font: out of memory");
                return {};
            }
        }

        cached_mmap->ref();
        cached_face->ref();
        return face
        {
            cached_face->face,
            faces.index(cached_face)
        };
    }

    face default_font() noexcept
    {
        static const auto cached_font = load_font(default_font_name, default_font_size);
        return clone(cached_font);
    }
}