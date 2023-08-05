#pragma once

#include <core/zstring_view.h>
#include <core/resource.h>

#include <ui/app.h>

namespace ui
{  
    extern event_result_t D_OS_APICALL window_procedure
    (
        window_handle_t window,
        uint_t message,
        word_parameter_t word_parameter,
        long_parameter_t long_parameter
    ) noexcept;

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

        type_window_builder& window_procedure(wndproc_t proc) noexcept;

        [[nodiscard]]
        module_handle_t module() const noexcept;

        [[nodiscard]]
        unique_type_window build_as(wzstring_view name) noexcept;

        [[nodiscard]] 
        unique_type_window build() noexcept;

    private:
        [[nodiscard]]
        tagWNDCLASSEXW* wndcls() noexcept;

        [[nodiscard]]
        const tagWNDCLASSEXW* cwndcls() const noexcept;

    private:
        static constexpr size_t wndclass_len{ 80u };
        static constexpr size_t wndclass_align{ 8u };
        alignas(wndclass_align) std::byte storage_[wndclass_len]{};
    };
}