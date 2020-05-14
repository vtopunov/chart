#pragma once

#include <string>

#include <core/handle.h>

#include <platform/windows/defs.h>

namespace os_windows
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

    struct window_type_view
    {
        HMODULE module_address;
        LPCWSTR name_id;
    };

    constexpr bool valid(window_type_view type) noexcept
    {
        return !!type.name_id;
    }

    bool close(private_handle_t, window_type_view type) noexcept;

    using safe_window_type = shared_handle<window_type_view>;

    class window_type_factory
    {
    public:
        window_type_factory() noexcept = default;

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

        constexpr window_type_factory& module_address(HMODULE module_address)
        {
            D_ASSERT(module_address);
            data_.hInstance = module_address;
            return *this;
        }

        constexpr window_type_factory& background(HBRUSH brush) noexcept
        {
            data_.hbrBackground = brush;
            return *this;
        }

        window_type_factory& background(stock_brush brush) noexcept;

        safe_window_type create() const noexcept;

    private:
        WNDCLASSEXW data_{};
        std::wstring name_;
    };
}