#pragma once

#include <core/underlying.h>

#include <file/file.h>

namespace file
{
    constexpr os::file_descriptor_t file_resource_to_native(file_resource file) noexcept
    {
        return underlying_cast<os::file_descriptor_t>(file.fd);
    }
}