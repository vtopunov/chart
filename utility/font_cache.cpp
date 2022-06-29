#include "font_cache.h"

#include <filesystem>

#include <utility/cache_storage.h>

#include <debug/debug.h>

#include <file/file_mmap.h>

namespace font_cache
{
    namespace
    {
        struct mmap_item
        {
            file::path_string name;
            file::file_mmap mmap;

            struct by_name
            {
                file::path_string_view name;
                
                [[nodiscard]]
                constexpr bool operator () (const mmap_item& item) const noexcept
                {
                    return name == item.name;
                }
            };
        };

        struct face_item
        {
            font::face face;
            size_t mmap_id;
            px::pxside_t size;

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
                pxside_t size;

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

        mmaps_cache& global_mmaps_cache() noexcept
        {
            static mmaps_cache cache{};
            return cache;
        }

        faces_cache& global_faces_cache() noexcept
        {
            static faces_cache cache{};
            return cache;
        }
    }

    void cache_deref::operator()(const font::face_descriptor_t face) const noexcept
    {
        if (face)
        {
            auto& mmaps = global_mmaps_cache();
            auto& faces = global_faces_cache();

            const auto item = faces.select(face_item::by_face{ face });
            D_ASSERT(item);
            item->deref(faces);

            const auto mmap_id = item->mmap_id;
            D_ASSERT(mmap_id < mmaps.size());
            mmaps[mmap_id].deref(mmaps);
        }
    }
    
    font_cache::face load_font(file::path_string_view name, const px::pxside_t size) noexcept
    {
        auto& mmaps = global_mmaps_cache();
        auto& faces = global_faces_cache();

        auto cached_mmap = mmaps.select(mmap_item::by_name{ name });

        faces_pointer cached_face{ nullptr };
        if(cached_mmap)
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
                if (!font::size(garbage->face, size))
                {
                    return {};
                }
                garbage->size = size;
                cached_face = garbage;
            }
        }

        if(!cached_face)
        {
            const_buffer_view font_storage{};
            file::path_string file_name;
            file::file_mmap file_mmap;

            if (cached_mmap)
            {
                font_storage = cached_mmap->mmap;
            }
            else
            {
                file_name = name;

                file_mmap = file::mmap(file_name);
                if (!file_mmap)
                {
                    e_debug(_PATH("can't open font file: {}"), file_name);
                    return {};
                }

                font_storage = file_mmap;
            }

            auto face = font::create_face(font_storage, size);
            if (!face)
            {
                return {};
            }

            if (!cached_mmap)
            {
                cached_mmap = mmaps.try_emplace(std::move(file_name), std::move(file_mmap));
                if (!cached_mmap)
                {
                    e_debug("load_font: out of memory");
                    return {};
                }
            }

            cached_face = faces.try_emplace(std::move(face), mmaps.index(cached_mmap), size);
            if (!cached_face)
            {
                e_debug("load_font: out of memory");
                return {};
            }
        }

        cached_mmap->ref();
        cached_face->ref();
        return
        {
            resource_construct,
            cached_face->face
        };
    }
}