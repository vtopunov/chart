#pragma once

#include <core/zstring_view.h>
#include <core/resource.h>
#include <core/color.h>

#include <ui/app.h>


namespace ui
{  
    struct type_window_parameters;

    struct gdi_object_deleter
    {
        void operator () (gdi_object_handle_t o) const noexcept;
    };
    
    using unique_brush = unique_resource<brush_handle_t, gdi_object_deleter>;

    unique_brush create_brush(rgba_color32_t color) noexcept;

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
        type_window_builder() noexcept;

        type_window_builder(const type_window_builder& builder) noexcept;

        ~type_window_builder() noexcept;

        type_window_builder(type_window_builder&&) noexcept = delete;

        type_window_builder& operator = (const type_window_builder& builder) noexcept;

        type_window_builder& operator = (type_window_builder&&) noexcept = delete;

        type_window_builder& style(uint_t style) noexcept;

        type_window_builder& module(module_handle_t module) noexcept;

        type_window_builder& background(stock_brush brush) noexcept;

        type_window_builder& background(unique_brush brush) noexcept;

        type_window_builder& window_procedure(wndproc_t proc) noexcept;

        type_window_builder& add_style(uint_t new_style) noexcept
        {
            return style(style() | new_style);
        }

        [[nodiscard]]
        uint_t style() const noexcept;

        [[nodiscard]]
        module_handle_t module() const noexcept;

        [[nodiscard]]
        const_brush_handle_t background() const noexcept;

        [[nodiscard]]
        unique_type_window build_as(wzstring_view name) noexcept;

        [[nodiscard]] 
        unique_type_window build() noexcept;

    private:
        [[nodiscard]]
        type_window_parameters* _p_params() noexcept;

        [[nodiscard]]
        const type_window_parameters* _c_p_params() const noexcept;

    private:
        static constexpr size_t param_len{ 104u };
        static constexpr size_t param_align{ 8u };
        alignas(param_align) std::byte storage_[param_len]{};
    };
}