#pragma once

#include <optional>

#include <core/rect.h>
#include <core/small_flat_map.h>

#include <platform/windows/window_type.h>


namespace os_windows
{
    namespace native_window_system
    {
        struct childrens_enumerator
        {
            const_key_value_range_t<const_window_handle_t, window_view> range;
            const_window_handle_t key;

            explicit constexpr operator bool() const noexcept
            {
                return is_valid();
            }

            constexpr childrens_enumerator begin() const noexcept
            {
                return *this;
            }

            constexpr empty end() const noexcept
            {
                return {};
            }

            constexpr bool is_valid() const noexcept
            {
                return starts_with_key(range, key);
            }

            constexpr bool operator == (empty) const noexcept
            {
                return !is_valid();
            }

            constexpr bool operator != (empty) const noexcept
            {
                return is_valid();
            }

            constexpr childrens_enumerator& operator++() noexcept
            {
                ++range.first;
                return *this;
            }

            constexpr window_view operator*() const noexcept
            {
                return range.first->value;
            }
        };

        void close_childrens(const_window_handle_t parent) noexcept;

        childrens_enumerator childrens(const_window_handle_t parent) noexcept;

        rect_t full_rect(window_handle_t window_handle) noexcept;

        rect_t client_rect(window_handle_t window_handle) noexcept;

        class window_data
        {
        public:
            using view_type = window_view;

            constexpr window_data() noexcept = default;

            window_data(window_view window, safe_window_type type) noexcept
                : window_{ window }
                , type_{ std::move(type) }
            {}

            constexpr operator view_type () const noexcept
            {
                return window_;
            }

            constexpr window_handle_t handle() const noexcept
            {
                return window_.handle;
            }

            safe_window_type type() const noexcept
            {
                return type_;
            }

        private:
            window_view window_{ null_window };
            safe_window_type type_;
        };

        using safe_window = shared_handle<window_data>;
    }

    using childrens_window_enumerator = native_window_system::childrens_enumerator;
    using safe_window = native_window_system::safe_window;

    bool exist(window_view window) noexcept;

    bool show(window_view window, int cmd = SW_SHOW) noexcept;

    bool update(window_view window) noexcept;

    bool close(window_view window) noexcept; 

    rect_t full_rect(window_view window) noexcept;

    rect_t client_rect(window_view window) noexcept;

    childrens_window_enumerator childrens(window_view window) noexcept;

    class window_factory
    {
    public:
        window_factory() noexcept = default;

        window_factory& type(safe_window_type type) noexcept
        {
            type_ = std::move(type);
            return *this;
        }

        window_factory& title(std::wstring title) noexcept
        {
            title_ = std::move(title);
            return *this;
        }

        constexpr window_factory& parent(window_view window) noexcept
        {
            parent_ = window;
            return *this;
        }

        constexpr window_factory& position(point_t position) noexcept
        {
            position_ = position;
            return *this;
        }

        constexpr window_factory& position(pixel_t x, pixel_t y) noexcept
        {
            return position(make_point(x, y));
        }

        constexpr window_factory& rect(const rect_t& rc)
        {
            return position(rc.v00()).size(rc.size());
        }

        constexpr window_factory& size(rect_size_t size)  noexcept
        {
            size_ = size;
            return *this;
        }

        constexpr window_factory& size(pixel_t width, pixel_t height)  noexcept
        {
            return size(make_rect_size(width, height));
        }

        safe_window create() const noexcept;

    private:
        safe_window_type type_;
        std::wstring title_;
        window_view parent_{ null_window };
        std::optional<DWORD> style_;
        point_t position_{ use_default_position };
        rect_size_t size_{ use_default_size };
    };
}