#pragma once

#include <optional>

#include <core/underlying_cast.h>
#include <core/rect.h>
#include <core/small_vector.h>

#include <ui/window_fwd.h>
#include <ui/window_type.h>

namespace ui
{   
    inline constexpr auto usedefault_px = narrow_cast<pixel_t>(CW_USEDEFAULT);
    inline constexpr auto usedefault_upx = static_cast<upixel_t>(CW_USEDEFAULT);
    inline constexpr auto usedefault_vec = fill_vec2(usedefault_px);
    inline constexpr size2d_t usedefault_size2d{ fill_vec2(usedefault_upx) };

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

    using window_container = small_vector<window_dependency, 6>;

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
#pragma warning(push)
#pragma warning(disable : 26446) // Prefer to use gsl::at() instead of unchecked subscript operator
            return (*contaner)[position];
#pragma warning(pop)
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
    rect_px_t rect(window_resource window) noexcept;

    bool rect(window_resource window, rect_px_t rc) noexcept;

    [[nodiscard]]
    size2d_t desktop_sizes() noexcept;

    [[nodiscard]]
    size2d_t display_resolution() noexcept;

    using window_t = unique_resource<window_resource>;

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

        constexpr window_factory& position(vec2px_t position) noexcept
        {
            position_ = position;
            return *this;
        }


        constexpr window_factory& position(pixel_t x, pixel_t y) noexcept
        {
            return position(vec2px_t{ x, y });
        }

        constexpr window_factory& sizes(size2d_t sizes) noexcept
        {
            sizes_._0 = narrow_cast<pixel_t>(sizes.width());
            sizes_._1 = narrow_cast<pixel_t>(sizes.height());
            return *this;
        }

        constexpr window_factory& sizes(upixel_t width, upixel_t height) noexcept
        {
            return sizes(size2d_t{ width, height });
        }

        constexpr window_factory& rect(const rect_px_t& rc) noexcept
        {
            return position(rc.p00()).sizes(rc.sizes());
        }


        [[nodiscard]]
        window_t create() noexcept;

    private:
        static constexpr auto usedefault_vec = fill_vec2(narrow_cast<pixel_t>(CW_USEDEFAULT));

    private:
        shared_window_type_t type_;
        std::wstring title_;
        window_resource parent_ = nullwindow;
        std::optional<DWORD> style_;
        vec2px_t position_{ usedefault_vec };
        vec2px_t sizes_{ usedefault_vec };
    };
}