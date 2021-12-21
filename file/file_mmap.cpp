#include "file_mmap.h"

#include <core/utility.h>

#include <core/os.h>

namespace file
{
    void file_mmap_resource_deleter::operator()(file_mmap_resource resource) const noexcept
    {
        const auto& p = resource.private_detail_;

        if ( p.data_ )
        {
            UnmapViewOfFile(p.data_);
        }

        if ( p.fmmd_ )
        {
            CloseHandle(p.fmmd_);
        }

        constexpr file_resource_deleter close{};
        close(p.file_);
    }

    file_mmap mmap(path_string_view_t path) noexcept
    {
        file_mmap result;

        auto& p = as_mutable(result.r().private_detail_);

        p.file_ = ro_open(path).release();

        if ( p.file_ != invalidfile )
        {
            p.size_ = clamp_cast<size_t>(size(p.file_));
        }

        if ( p.size_ )
        {
            p.fmmd_ = CreateFileMappingW
            (
                p.file_.fd,
                nullptr,
                PAGE_READONLY,
                hi_cast<DWORD>( p.size_ ),
                lo_cast<DWORD>( p.size_ ),
                nullptr
            );
        }

        if ( p.fmmd_ )
        {
            p.data_ = MapViewOfFile
            (
                p.fmmd_,
                FILE_MAP_READ, 
                0u, 
                0u, 
                safe_numeric_cast<SIZE_T>(p.size_)
            );
        }

        if ( !p.data_ )
        {
            result.reset();
        }

        return result;
    }
}


