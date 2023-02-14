#pragma once

#include <core/zstring_view.h>
#include <core/resouce.h>

#include <ui/app.h>

namespace ui
{    
    enum class stock_brush
    {
        white,
        light_gray,
        gray,
        dark_gray,
        black,
        null
    };

    struct type_window_resource
    {
        using name_id_t = const wchar_t*;

        module_handle_t module;
        name_id_t name_id;

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
        type_window_builder& style(uint_t style) noexcept;

        type_window_builder& module(module_handle_t module) noexcept;

        type_window_builder& background(stock_brush brush) noexcept;

        [[nodiscard]]
        module_handle_t module() const noexcept;

        [[nodiscard]]
        unique_type_window build_as(wzstring_view name) noexcept;

        [[nodiscard]] 
        unique_type_window build() noexcept;

    private:
        tagWNDCLASSEXW* wndcls() noexcept;

        const tagWNDCLASSEXW* cwndcls() const noexcept;

    private:
        static constexpr size_t wndclass_len{ 80u };
        static constexpr size_t wndclass_align{ 8u };
        alignas(wndclass_align) std::byte storage_[wndclass_len]{};
    };
}