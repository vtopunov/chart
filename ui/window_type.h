#pragma once

#include <string>

#include <core/resouce.h>
#include <core/os.h>

namespace ui
{
    using error_code_t = DWORD;

    [[nodiscard]]
    error_code_t error_code() noexcept;
    
    enum class stock_brush : int
    {
        white = WHITE_BRUSH,
        light_gray = LTGRAY_BRUSH,
        gray = GRAY_BRUSH,
        dark_gray = DKGRAY_BRUSH,
        black = BLACK_BRUSH,
        null = NULL_BRUSH,
        hollow = HOLLOW_BRUSH
    };

    [[nodiscard]]
    HBRUSH stock(stock_brush brush) noexcept;

    struct window_type_resource
    {
        HMODULE module_instance;
        LPCWSTR name_id;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!name_id;
        }
    };

    using nullwindowtype_t = null_t<window_type_resource>;

    inline constexpr nullwindowtype_t nullwindowtype{};

    struct window_type_resource_deleter
    {
        void operator () (window_type_resource type, resource_destroy_t) const noexcept;
    };

    using unique_window_type_t = unique_resource<window_type_resource, window_type_resource_deleter>;

    using shared_window_type_t = shared_resource<window_type_resource, window_type_resource_deleter>;

    class window_type_factory
    {
    public:
        constexpr window_type_factory& style(UINT style) noexcept
        {
            data_.style = style;
            return *this;
        }

        window_type_factory& name(std::wstring name) noexcept
        {
            name_ = std::move(name);
            data_.lpszClassName = name_.c_str();
            return *this;
        }

        constexpr window_type_factory& module_instance(HMODULE instance) noexcept
        {
            data_.hInstance = instance;
            return *this;
        }

        window_type_factory& background(stock_brush brush) noexcept
        {
            data_.hbrBackground = stock(brush);
            return *this;
        }

        [[nodiscard]]
        unique_window_type_t create() noexcept;

    private:
        WNDCLASSEXW data_{};
        std::wstring name_;
    };
}