#pragma once

struct android_app;

namespace common
{
    struct app_own
    {
        explicit app_own(android_app* am) noexcept;

        static android_app* release() noexcept;

        ~app_own() noexcept;
    };

    [[nodiscard]]
    android_app* app() noexcept;
}