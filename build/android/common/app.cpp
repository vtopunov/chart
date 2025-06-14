#include "app.h"


namespace common
{
    namespace
    {
        android_app* app_{ nullptr };
    }

    android_app* app() noexcept
    {
        return app_;
    }
    
    app_own::app_own(android_app* am) noexcept
    {
        app_ = am;
    }

    android_app* app_own::release() noexcept
    {
        const auto temp = app_;
        app_ = nullptr;
        return temp;
    }
    
    app_own::~app_own() noexcept
    {
        app_ = nullptr;
    }
}