#pragma once

#include <core/zstring_view.h>
#include <core/resouce.h>

#include <os/os.h>

#include <ui/app.h>

namespace ui
{    
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

    struct type_window_resource
    {
        module_handle_t module;
        LPCWSTR name_id;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!name_id;
        }
    };

    using nulltypewindow_t = null_t<type_window_resource>;

    constexpr nulltypewindow_t nullwindowtype{};

    struct window_type_resource_deleter
    {
        void operator () (type_window_resource type) const noexcept;
    };

    using unique_type_window = unique_resource<type_window_resource, window_type_resource_deleter>;

    using shared_type_window = shared_resource<type_window_resource, window_type_resource_deleter>;

    class type_window_builder
    {
    public:
        constexpr type_window_builder& style(UINT style) noexcept
        {
            data_.style = style;
            return *this;
        }

        constexpr type_window_builder& module(module_handle_t module) noexcept
        {
            data_.hInstance = module;
            return *this;
        }

        type_window_builder& background(stock_brush brush) noexcept
        {
            data_.hbrBackground = stock(brush);
            return *this;
        }

        [[nodiscard]]
        unique_type_window build_as(wzstring_view name) noexcept;

        [[nodiscard]] 
        unique_type_window build() noexcept;

    private:
        WNDCLASSEXW data_{};
    };
}