#pragma once

#include <os/os_detection.h>

#ifdef D_OS_WINDOWS
#include <string>

#include <core/small_vector.h>

#include <ui/type_window.h>

#else
#include <ui/fwd.h>

#endif


namespace ui
{
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

    struct window_parameters
    {
        mutable type_window_builder type_builder{};
        mutable shared_type_window cached_type{};
        std::wstring title{};
        pxrectangle geometry{ rc_usedefault };
        window_handle_t parent{ nullptr };
    };

    inline void prepare(const window_parameters& params) noexcept
    {
        if (params.cached_type)
        {
            if (!params.type_builder.module())
            {
                params.type_builder.module(params.cached_type.r().module);
            }
        }
        else
        {
            params.cached_type = params.type_builder.build();
        }

        D_ASSERT(!params.cached_type || (params.cached_type.r().module == params.type_builder.module()));
    }

    [[nodiscard]]
    inline module_handle_t mutable_app_module_handle(const window_parameters& param) noexcept
    {
        return param.type_builder.module();
    }

    [[nodiscard]]
    window create_window(const window_parameters& params) noexcept;

    template<class Builder, class Params>
    class window_gatherer
    {
    public:
        static_assert(std::is_base_of_v<window_parameters, Params>);

        Builder& type(unique_type_window type) noexcept
        {
            params_.cached_type = std::move(type);
            return _builder();
        }

        Builder& title(std::wstring title) noexcept
        {
            params_.title = std::move(title);
            return _builder();
        }

        constexpr Builder& parent(window_handle_t window) noexcept
        {
            params_.parent = window;
            return _builder();
        }

        constexpr Builder& position(pxpoint2d position) noexcept
        {
            params_.geometry.position = position;
            return _builder();
        }

        constexpr Builder& position(pxsize_t x, pxsize_t y) noexcept
        {
            return position(pxpoint2d{ x, y });
        }

        constexpr Builder& sizes(pxsize2d sizes) noexcept
        {
            params_.geometry.sizes = sizes;
            return _builder();
        }

        constexpr Builder& sizes(pxsize_t width, pxsize_t height) noexcept
        {
            return sizes(pxsize2d{ width, height });
        }

        constexpr Builder& geometry(const pxrectangle& rc) noexcept
        {
            params_.geometry = rc;
            return _builder();
        }

        Builder& module(module_handle_t module) noexcept
        {
            params_.type_builder.module(module);
            return _builder();
        }

        Builder& background(stock_brush brush) noexcept
        {
            params_.type_builder.background(brush);
            params_.cached_type.deattach_and_reset();
            return _builder();
        }

        Builder& background(unique_brush brush) noexcept
        {
            params_.type_builder.background(std::move(brush));
            params_.cached_type.deattach_and_reset();
            return _builder();
        }

        Builder& window_procedure(wndproc_t proc) noexcept
        {
            params_.type_builder.window_procedure(proc);
            params_.cached_type.deattach_and_reset();
            return _builder();
        }

        [[nodiscard]]
        module_handle_t module() const noexcept
        {
            return mutable_app_module_handle(_c_params());
        }

        [[nodiscard]]
        const_brush_handle_t background() const noexcept
        {
            return params_.type_builder.background();
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

    struct window_builder : window_gatherer<window_builder, window_parameters>
    {
        [[nodiscard]]
        window build() const noexcept
        {
            return create_window(_c_params());
        }
    };

#else
    [[nodiscard]]
    pxsize2d sizes(window_handle_t window) noexcept;

    using window_parameters = module_handle_t;

    [[nodiscard]]
    constexpr module_handle_t mutable_app_module_handle(window_parameters param) noexcept
    {
        return param;
    }

    template<class Builder, class Params>
    class window_gatherer
    {
    public:
        constexpr Builder& module(module_handle_t module) noexcept
        {
            params_ = module;
            return _builder();
        }

        [[nodiscard]]
        constexpr module_handle_t module() const noexcept
        {
            return params_;
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
        Params params_{ nullptr };
    };

#endif
}