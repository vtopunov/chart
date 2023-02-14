#pragma once

#include <string>

#include <core/rectangle.h>
#include <core/small_vector.h>

#include <px/fwd.h>

#include <ui/show_command.h>
#include <ui/type_window.h>


namespace ui
{
    struct window_dependency
    {
        window_handle_t current;
        window_handle_t parent;
        shared_type_window type;

        constexpr operator window_handle_t() const noexcept
        {
            return current;
        }

        constexpr bool is_root() const noexcept
        {
            return !parent;
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
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_unchecked_subscript_operator);
            return (*contaner)[position];
            D_WARNING_POP;
        }

        [[nodiscard]]
        constexpr reference operator*() const noexcept
        {
            return value();
        }
    };

    [[nodiscard]]
    window_childrens childrens(window_handle_t parent) noexcept;

    struct window_roots
    {
        using container_type = window_container;
        using reference = container_type::const_reference;

        size_t position;
        const container_type* contaner;

        [[nodiscard]]
        constexpr window_roots begin() const noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr null_t<window_roots> end() const noexcept
        {
            return {};
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return position < std::size(*contaner);
        }

        constexpr window_roots& operator++() noexcept
        {
            while ((++position) < std::size(*contaner))
            {
                if (value().is_root())
                    break;
            }

            return *this;
        }

        [[nodiscard]]
        constexpr reference value() const noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_unchecked_subscript_operator);
            return (*contaner)[position];
            D_WARNING_POP;
        }

        [[nodiscard]]
        constexpr reference operator*() const noexcept
        {
            return value();
        }
    };

    [[nodiscard]]
    window_roots roots() noexcept;

    bool show(window_handle_t window, int cmd) noexcept;

    inline bool show(window_handle_t window, show_command cmd) noexcept
    {
        return show(window, to_underlying(cmd));
    }

    inline bool show(window_handle_t window) noexcept
    {
        return show(window, show_command::show);
    }

    bool close(window_handle_t window) noexcept;

    [[nodiscard]]
    pxrectangle geometry(window_handle_t window) noexcept;

    [[nodiscard]]
    pxsize2d sizes(window_handle_t window) noexcept;

    bool geometry(window_handle_t window, pxrectangle rc) noexcept;

    [[nodiscard]]
    pxsize2d desktop_sizes() noexcept;

    struct window_resource_deleter
    {
        void operator () (window_handle_t window) const noexcept
        {
            close(window);
        }
    };

    using window = unique_resource<window_handle_t, window_resource_deleter>;

    namespace private_detail_window
    {
        using native_px_t = int;
        
        constexpr auto cw_usedefault = static_cast<native_px_t>(0x80000000);
        constexpr auto px_usedefault = static_cast<pxside_t>(cw_usedefault);
    }

    using private_detail_window::px_usedefault;

    class window_builder
    {
    public:
        window_builder& type(unique_type_window type) noexcept
        {
            cached_type_ = std::move(type);
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

        constexpr window_builder& position(pxpoint2d position) noexcept
        {
            geometry_.position = position;
            return *this;
        }

        constexpr window_builder& position(pxside_t x, pxside_t y) noexcept
        {
            return position(pxpoint2d{ x, y });
        }

        constexpr window_builder& sizes(pxsize2d sizes) noexcept
        {
            geometry_.sizes = sizes;
            return *this;
        }

        constexpr window_builder& sizes(pxside_t width, pxside_t height) noexcept
        {
            return sizes(pxsize2d{ width, height });
        }

        constexpr window_builder& geometry(const pxrectangle& rc) noexcept
        {
            geometry_ = rc;
            return *this;
        }

        window_builder& module(module_handle_t module) noexcept
        {
            type_builder_.module(module);
            return *this;
        }

        window_builder& background(stock_brush brush) noexcept
        {
            type_builder_.background(brush);
            cached_type_.deattach_and_reset();
            return *this;
        }

        [[nodiscard]]
        module_handle_t module() const noexcept
        {
            return type_builder_.module();
        }

        [[nodiscard]]
        window build() const noexcept;

    private:
        mutable type_window_builder type_builder_{};
        mutable shared_type_window cached_type_{};
        std::wstring title_;
        pxrectangle geometry_{ px_usedefault, 0_px, px_usedefault, 0_px };
        window_handle_t parent_{ nullptr };
    };
}