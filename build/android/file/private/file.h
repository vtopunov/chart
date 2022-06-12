#pragma once

#include <file/file.h>

#include <core/underlying.h>

namespace file
{
    constexpr os::file_descriptor_t file_resource_to_native(file_resource file) noexcept
    {
        return underlying_cast<os::file_descriptor_t>(file.fd);
    }
}