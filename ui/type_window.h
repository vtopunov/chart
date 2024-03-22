#pragma once

#include <core/zstring_view.h>

#include <ui/app.h>
#include <ui/window_procedure.h>
#include <ui/brush.h>


namespace ui
{
    struct type_window_parameters;

    using type_window_handle_t = D_CONDITIONAL_OS_ANDROID(sensor_event_queue_resource, const wchar_t*);

    struct type_window_resource
    {
        type_window_handle_t handle;
        module_handle_t module;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!handle;
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
        unique_type_window build(wzstring_view name) noexcept;

        [[nodiscard]]
        unique_type_window build() noexcept;

#ifdef D_OS_WINDOWS
    private:
        [[nodiscard]]
        type_window_parameters* _p_impl() noexcept;

        [[nodiscard]]
        const type_window_parameters* _c_p_impl() const noexcept;

    private:
        static constexpr size_t storage_size{ 104u };
        static constexpr size_t storage_align{ 8u };
        alignas(storage_align) std::byte storage_[storage_size]{};
#else
    private:
        module_handle_t module_;
#endif
    };
}