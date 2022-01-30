#pragma once

#include <optional>

#include <core/rect.h>
#include <core/small_vector.h>
#include <core/zstring_view.h>

#include <px/pxfwd.h>

#include <ui/window_fwd.h>
#include <ui/window_type.h>

namespace ui
{
    struct window_dependency
    {
        window_resource current;
        window_resource parent;
        shared_window_type_t type;

        constexpr operator window_resource() const noexcept
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
        window_resource parent;

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
    window_childrens childrens(window_resource parent) noexcept;

    bool show(window_resource window, int cmd) noexcept;

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

    inline bool show(window_resource window, show_command cmd) noexcept
    {
        return show(window, to_underlying(cmd));
    }

    inline bool show(window_resource window) noexcept
    {
        return show(window, show_command::show);
    }

    bool update(window_resource window) noexcept;

    bool close(window_resource window) noexcept;

    void quit() noexcept;

    [[nodiscard]]
    px::rect_t rect(window_resource window) noexcept;

    bool rect(window_resource window, px::rect_t rc) noexcept;

    [[nodiscard]]
    px::size2d_t desktop_sizes() noexcept;

    [[nodiscard]]
    px::size2d_t display_resolution() noexcept;

    struct window_resource_deleter
    {
        void operator () (window_resource window) const noexcept
        {
            close(window);
        }
    };

    using window_t = unique_resource<window_resource, window_resource_deleter>;

    class window_factory
    {
    public:
        window_factory& type(unique_window_type_t type) noexcept
        {
            type_ = std::move(type);
            return *this;
        }

        window_factory& title(std::wstring title) noexcept
        {
            title_ = std::move(title);
            return *this;
        }

        constexpr window_factory& parent(window_resource window) noexcept
        {
            parent_ = window;
            return *this;
        }

        constexpr window_factory& position(px::point2d_t position) noexcept
        {
            position_ = position;
            return *this;
        }

        constexpr window_factory& position(pxside_t x, pxside_t y) noexcept
        {
            return position(px::point2d_t{ x, y });
        }

        constexpr window_factory& sizes(px::size2d_t sizes) noexcept
        {
            sizes_ = sizes;
            return *this;
        }

        constexpr window_factory& sizes(pxside_t width, pxside_t height) noexcept
        {
            return sizes(px::size2d_t{ width, height });
        }

        constexpr window_factory& rect(const px::rect_t& rc) noexcept
        {
            return position(rc.p00()).sizes(rc.sizes());
        }


        [[nodiscard]]
        window_t create() noexcept;

    private:
        using native_pxside_t = int;
        static constexpr native_pxside_t cw_usedefault{ CW_USEDEFAULT };
        static constexpr auto px_usedefault = static_cast<pxside_t>(cw_usedefault);

        static constexpr native_pxside_t px_to_native(pxside_t px) noexcept
        {
            return (px == px_usedefault) ? cw_usedefault : narrow_cast<native_pxside_t>(px);
        }

    private:
        shared_window_type_t type_;
        std::wstring title_;
        window_resource parent_ = nullwindow;
        std::optional<DWORD> style_;
        px::point2d_t position_{ px_usedefault, 0_px };
        px::size2d_t sizes_{ px_usedefault, 0_px };
    };
}