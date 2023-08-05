#pragma once

#include <string>

#include <core/small_vector.h>

#include <ui/window_constants.h>
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
    };

    using window_set = small_vector<window_dependency, 6_uz>;

    struct siblings_window
    {
        using const_reference = window_set::const_reference;

        size_t position;
        const window_set* storage;
        window_handle_t parent;

        [[nodiscard]]
        constexpr siblings_window begin() const noexcept
        {
            return *this;
        }

        struct end_t {};

        [[nodiscard]]
        constexpr end_t end() const noexcept
        {
            return {};
        }

        [[nodiscard]]
        constexpr bool has_value() const noexcept
        {
            return position < std::size(*storage) && parent == value().parent;
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return has_value();
        }

        [[nodiscard]]
        constexpr bool operator != (end_t) const noexcept
        {
            return has_value();
        }

        constexpr siblings_window& operator++() noexcept
        {
            ++position;
            return *this;
        }

        [[nodiscard]]
        constexpr const_reference value() const noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_unchecked_subscript_operator);
            return (*storage)[position];
            D_WARNING_POP;
        }

        [[nodiscard]]
        constexpr const_reference operator*() const noexcept
        {
            return value();
        }
    };

    [[nodiscard]]
    siblings_window childrens(window_handle_t parent) noexcept;

    [[nodiscard]]
    siblings_window roots() noexcept;

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

    bool window_text(window_handle_t window, wzstring_view text) noexcept;

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

        window_builder& window_procedure(wndproc_t proc) noexcept
        {
            type_builder_.window_procedure(proc);
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
        pxrectangle geometry_{ rc_usedefault };
        window_handle_t parent_{ nullptr };
    };
}