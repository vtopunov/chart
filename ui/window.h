#pragma once

#include <string>

#include <core/rect.h>
#include <core/small_vector.h>

#include <px/pxfwd.h>

#include <ui/type_window.h>

namespace ui
{
    static_assert(std::is_same_v<window_handle_t, HWND>);

    struct window_dependency
    {
        window_handle_t current;
        window_handle_t parent;
        shared_type_window type;

        constexpr operator window_handle_t() const noexcept
        {
            return current;
        }
    };

    using window_container = small_vector<window_dependency, 6_uz>;

    struct window_childrens
    {
        using container_type = window_container;
        using reference = container_type::const_reference;

        size_t position;
        const container_type* contaner;
        window_handle_t parent;

        [[nodiscard]]
        constexpr window_childrens begin() const noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr null_t<window_childrens> end() const noexcept
        {
            return {};
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return position < std::size(*contaner) && parent == value().parent;
        }

        constexpr window_childrens& operator++() noexcept
        {
            ++position;
            return *this;
        }

        [[nodiscard]]
        constexpr reference value() const noexcept
        {
            D_WARNING_PUSH
                D_WARNING_DISABLE_MSVC(W_unchecked_subscript_operator)
                return (*contaner)[position];
            D_WARNING_POP
        }

        [[nodiscard]]
        constexpr reference operator*() const noexcept
        {
            return value();
        }
    };

    [[nodiscard]]
    window_childrens childrens(window_handle_t parent) noexcept;

    inline bool show(window_handle_t window, int cmd) noexcept
    {
        return !!::ShowWindow(window, cmd);
    }

    enum class show_command
    {
        hide = SW_HIDE,
        show = SW_SHOW,
        show_no_activete = SW_SHOWNOACTIVATE,
        show_minimazed = SW_SHOWMINIMIZED,
        show_maximazed = SW_SHOWMAXIMIZED,
        show_minimazed_no_activete = SW_SHOWMINNOACTIVE,
        restore = SW_RESTORE
    };

    inline bool show(window_handle_t  window, show_command cmd) noexcept
    {
        return show(window, to_underlying(cmd));
    }

    inline bool show(window_handle_t window) noexcept
    {
        return show(window, show_command::show);
    }

    bool close(window_handle_t window) noexcept;

    inline void quit() noexcept
    {
        ::PostQuitMessage(0);
    }

    [[nodiscard]]
    px::rect geometry(window_handle_t window) noexcept;

    bool geometry(window_handle_t window, px::rect rc) noexcept;

    [[nodiscard]]
    inline window_handle_t desktop_window() noexcept
    {
        return ::GetDesktopWindow();
    }

    [[nodiscard]]
    inline px::size2d desktop_sizes() noexcept
    {
        return geometry(desktop_window()).sizes();
    }

    [[nodiscard]]
    px::size2d display_resolution() noexcept;

    struct window_resource_deleter
    {
        void operator () (window_handle_t window) const noexcept
        {
            close(window);
        }
    };

    using window = unique_resource<window_handle_t, window_resource_deleter>;

    class window_builder
    {
    public:
        window_builder& type(unique_type_window type) noexcept
        {
            type_ = std::move(type);
            return *this;
        }

        window_builder& title(std::wstring title) noexcept
        {
            title_ = std::move(title);
            return *this;
        }

        constexpr window_builder& parent(window_handle_t window) noexcept
        {
            parent_ = window;
            return *this;
        }

        constexpr window_builder& position(px::point2d position) noexcept
        {
            position_ = position;
            return *this;
        }

        constexpr window_builder& position(pxside_t x, pxside_t y) noexcept
        {
            return position(px::point2d{ x, y });
        }

        constexpr window_builder& sizes(px::size2d sizes) noexcept
        {
            sizes_ = sizes;
            return *this;
        }

        constexpr window_builder& sizes(pxside_t width, pxside_t height) noexcept
        {
            return sizes(px::size2d{ width, height });
        }

        constexpr window_builder& geometry(const px::rect& rc) noexcept
        {
            return position(rc.p00()).sizes(rc.sizes());
        }

        constexpr window_builder& module(module_handle_t module) noexcept
        {
            module_ = module;
            return *this;
        }

        [[nodiscard]]
        window build() noexcept;

    private:
        using native_pxside_t = int;
        static constexpr native_pxside_t cw_usedefault{ CW_USEDEFAULT };
        static constexpr auto px_usedefault = static_cast<pxside_t>(cw_usedefault);

        static constexpr native_pxside_t px_to_native(pxside_t px) noexcept
        {
            return (px == px_usedefault) ? cw_usedefault : narrow_cast<native_pxside_t>(px);
        }

    private:
        shared_type_window type_;
        std::wstring title_;
        std::optional<DWORD> style_;
        px::point2d position_{ px_usedefault, 0_px };
        px::size2d sizes_{ px_usedefault, 0_px };
        window_handle_t parent_{ nullptr };
        module_handle_t module_{ nullptr };
    };
}