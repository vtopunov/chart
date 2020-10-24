#pragma once

#include <optional>

#include <core/resouce.h>

#include <display/defs.h>

namespace display
{
    namespace event_processor
    {
        enum class processor_resource : size_t
        {};

        using nullprocessor_t = null_t<processor_resource>;

        inline constexpr nullprocessor_t nullprocessor{};

        bool close(processor_resource processor) noexcept;

        using processor_t = unique_resource<processor_resource>;

        [[nodiscard]]
        processor_t bind(window_resource window, void* data, event_callback_t callback) noexcept;
    }
}