#pragma once

#include <os/os_detection.h>

#ifdef D_OS_WINDOWS
#include <string>
#include <core/small_vector.h>
#endif

#include <ui/type_window.h>


namespace ui
{
    using title_string_t = D_OS_WINDOWS_OR(std::wstring, wzstring_view);

    struct window_parameters
    {
        mutable type_window_builder type_builder{};

#ifdef D_OS_WINDOWS
        title_string_t title{};
        pxrectangle geometry{ rc_usedefault };
        window_handle_t parent{ nullptr };
#endif
    };

    template<class Builder, class Params>
    class window_gatherer
    {
    public:
        static_assert(std::disjunction_v<std::is_same<window_parameters, Params>, std::is_base_of<window_parameters, Params>>);

        Builder& background(stock_brush brush) noexcept
        {
            params_.type_builder.background(brush);
            return _builder();
        }

        Builder& background(unique_brush brush) noexcept
        {
            params_.type_builder.background(std::move(brush));
            return _builder();
        }

        Builder& window_procedure(wndproc_t proc) noexcept
        {
            params_.type_builder.window_procedure(proc);
            return _builder();
        }

        [[nodiscard]]
        const_brush_handle_t background() const noexcept
        {
            return params_.type_builder.background();
        }

        Builder& title(title_string_t title) noexcept
        {
            D_OS_WINDOWS_OR(params_.title = std::move(title), D_UNUSED(title));
            return _builder();
        }

        constexpr Builder& parent(window_handle_t parent_window) noexcept
        {
            D_OS_WINDOWS_OR(params_.parent = parent_window, D_UNUSED(parent_window));
            return _builder();
        }

        constexpr Builder& position(pxpoint position) noexcept
        {
            D_OS_WINDOWS_OR(params_.geometry.position = position, D_UNUSED(position));
            return _builder();
        }

        constexpr Builder& position(npx_t x, npx_t y) noexcept
        {
            return position(pxpoint{ x, y });
        }

        constexpr Builder& sizes(pxsizes sizes) noexcept
        {
            D_OS_WINDOWS_OR(params_.geometry.sizes = sizes, D_UNUSED(sizes));
            return _builder();
        }

        constexpr Builder& sizes(npx_t width, npx_t height) noexcept
        {
            return sizes(pxsizes{ width, height });
        }

        constexpr Builder& geometry(const pxrectangle& rc) noexcept
        {
            D_OS_WINDOWS_OR(params_.geometry = rc, D_UNUSED(rc));
            return _builder();
        }

    protected:
        [[nodiscard]]
        constexpr const Params& _c_params() const noexcept
        {
            return params_;
        }

        [[nodiscard]]
        constexpr Params& _params() noexcept
        {
            return params_;
        }

        [[nodiscard]]
        constexpr Builder& _builder() noexcept
        {
            static_assert(std::is_base_of_v<window_gatherer, Builder>);
            return static_cast<Builder&>(*this);
        }

    private:
        Params params_{};
    };


#ifdef D_OS_WINDOWS
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

    bool close(window_handle_t window) noexcept;

    bool window_text(window_handle_t window, wzstring_view text) noexcept;

    [[nodiscard]]
    pxrectangle geometry(window_handle_t window) noexcept;

    [[nodiscard]]
    pxsizes adjust_sizes(pxsizes sizes) noexcept;

    bool geometry(window_handle_t window, pxrectangle rc) noexcept;

    [[nodiscard]]
    pxsizes desktop_sizes() noexcept;

    struct window_resource_deleter
    {
        void operator () (window_handle_t window) const noexcept
        {
            close(window);
        }
    };

    using window = unique_resource<window_handle_t, window_resource_deleter>;

#else
    constexpr bool show(window_handle_t, int) noexcept
    {
        return false;
    }

    [[nodiscard]] constexpr pxsizes adjust_sizes(pxsizes sizes) noexcept
    {
        return sizes;
    }

    struct window
    {
        unique_type_window type{};
        window_handle_t handle{ nullptr };

        constexpr operator window_handle_t () const noexcept
        {
            return handle;
        }

        constexpr explicit operator bool () const noexcept
        {
            return !!handle;
        }
    };

#endif

    [[nodiscard]]
    pxsizes sizes(window_handle_t window) noexcept;

    D_OS_WINDOWS_OR(inline, constexpr) bool show(window_handle_t window, show_command cmd) noexcept
    {
        return show(window, to_underlying(cmd));
    }

    D_OS_WINDOWS_OR(inline, constexpr) bool show(window_handle_t window) noexcept
    {
        return show(window, show_command::show);
    }

    [[nodiscard]]
    D_OS_WINDOWS_OR(inline, constexpr) pxsizes adjust_sizes(npx_t w, npx_t h) noexcept
    {
        return adjust_sizes(size2d{ w, h });
    }

    [[nodiscard]]
    window create_window(const window_parameters& params) noexcept;

    struct window_builder : window_gatherer<window_builder, window_parameters>
    {
        [[nodiscard]]
        window build() const noexcept
        {
            return create_window(_c_params());
        }
    };
}