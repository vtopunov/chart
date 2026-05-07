#pragma once

#include <core/zstring_view.h>

#include <ui/window_procedure.h>
#include <ui/brush.h>


namespace ui
{
    struct type_window_parameters;

    struct type_window_resource
    {
#ifdef D_OS_ANDROID
        sensor_manager_handle_t sensor_manager;
        sensor_event_queue_handle_t sensor_event_queue;
#else
        const wchar_t* handle;
#endif

        struct null_type
        {
            constexpr operator type_window_resource () const noexcept
            {
                return
                {
#ifdef D_OS_ANDROID
                    .sensor_manager{ nullptr },
                    .sensor_event_queue{ nullptr }
#else
                    .handle{ nullptr }
#endif
                };
            }
        };

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return D_OS_ANDROID_OR(sensor_event_queue, handle);
        }

    };

    using nulltypewindow_t = null_t<type_window_resource>;

    constexpr nulltypewindow_t nullwindowtype{};

    struct window_type_resource_deleter
    {
        void operator () (type_window_resource type) const noexcept;
    };

    using shared_type_window = shared_resource<type_window_resource, window_type_resource_deleter>;
    using unique_type_window = unique_resource<type_window_resource, window_type_resource_deleter>;
    using type_window = D_OS_WINDOWS_OR(shared_type_window, unique_type_window);

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
        const_brush_handle_t background() const noexcept;

        [[nodiscard]]
        type_window build() noexcept;

#ifdef D_OS_WINDOWS
    private:
        [[nodiscard]]
        type_window_parameters* _p_impl() noexcept;

        [[nodiscard]]
        const type_window_parameters* _c_p_impl() const noexcept;

    private:
        static constexpr size_t storage_size{ 128u };
        static constexpr size_t storage_align{ 8u };
        alignas(storage_align) std::byte storage_[storage_size]{};
#endif
    };
}