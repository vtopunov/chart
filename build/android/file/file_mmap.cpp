#include <file/file_mmap.h>

#include <sys/mman.h>

#include <core/clamp_cast.h>

#include "private/private_file.h"

namespace file
{
    
    namespace
    {
        const void* map_failed_as_nullptr(const void* result) noexcept
        {
            return (MAP_FAILED != result) ? result : nullptr;
        }
    }

    void file_mmap_resource::deleter_type::operator()(file_mmap_resource resource) const noexcept
    {
        const auto& p = resource.private_detail_;

        if (p.data_)
        {
            ::munmap(const_cast<void*>(p.data_), p.size_);
        }

        constexpr file_resource::deleter_type close{};
        close(p.file_);
    }

    file_mmap mmap(path_zstring_view path) noexcept
    {
        file_mmap result;

        auto& p = as_mutable(result.r().private_detail_);

        p.file_ = ro_open(path).release();

        if (invalidfile != p.file_)
        {
            p.size_ = clamp_cast<size_t>(size(p.file_));
        }

        if (p.size_)
        {
            p.data_ = map_failed_as_nullptr
            (
                ::mmap
                (
                    nullptr,
                    p.size_, 
                    PROT_READ, 
                    MAP_SHARED, 
                    file_resource_to_native(p.file_),
                    0
                )
            );
        }

        if (!p.data_)
        {
            result.reset();
        }

        return result;
    }
}