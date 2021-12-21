#pragma once

#include <ui/window.h>
#include <egl/config.h>

namespace egl
{
    namespace private_detail_egl_descriptor
    {
        enum class descriptor_type_id
        {
            display,
            surface,
            context
        };

        struct egl_base_descriptor
        {};

        template<class descriptor>
        using egl_base_descriptor_t = std::conditional_t<std::is_class_v<std::remove_pointer_t<descriptor>>, std::remove_pointer_t<descriptor>, egl_base_descriptor>;

        template<descriptor_type_id TypeId, class NativeDescriptor>
        struct descriptor_source : egl_base_descriptor_t<NativeDescriptor>
        {
            static constexpr auto type_id = TypeId;
        };

        template<descriptor_type_id TypeId, class NativeDescriptor>
        using egl_descriptor_t = copy_pointer_t<NativeDescriptor, descriptor_source<TypeId, NativeDescriptor>>;

        using display_descriptor_t = egl_descriptor_t<descriptor_type_id::display, EGLDisplay>;

        using surface_descriptor_t = egl_descriptor_t<descriptor_type_id::surface, EGLSurface>;

        using context_descriptor_t = egl_descriptor_t<descriptor_type_id::context, EGLContext>;
    }

    using display_descriptor_t = private_detail_egl_descriptor::display_descriptor_t;
    using surface_descriptor_t = private_detail_egl_descriptor::surface_descriptor_t;
    using context_descriptor_t = private_detail_egl_descriptor::context_descriptor_t;

    struct display_surface
    {
        display_descriptor_t display;
        surface_descriptor_t surface;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!surface;
        }
    };

    struct painting_collector
    {
        void operator () (display_surface surface) const noexcept
        {
            eglSwapBuffers(surface.display, surface.surface);
        }
    };

    using painting_t = unique_resource<display_surface, painting_collector>;

    struct null_window_resources;

    struct window_resources
    {
        using view_type = ui::window_resource;

        struct null_type
        {
            [[nodiscard]]
            constexpr operator window_resources() const noexcept
            {
                return window_resources
                {
                    .app_wnd = ui::nullwindow,
                    .renderer_wnd = ui::nullwindow,
                    .sizes{ 0_px, 0_px },
                    .display{ nullptr },
                    .surface{ nullptr },
                    .context{ nullptr }
                };
            }
        };

        view_type app_wnd;
        view_type renderer_wnd;

        px::size2d_t sizes;

        display_descriptor_t display;
        surface_descriptor_t surface;
        context_descriptor_t context;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!context;
        }

        [[nodiscard]]
        constexpr operator view_type () const noexcept
        {
            return app_wnd;
        }
    };

    struct window_resources_collector
    {
        void operator () (const window_resources& egl) const noexcept;
    };

    using window_t = unique_resource<window_resources, window_resources_collector>;

    [[nodiscard]]
    inline painting_t begin_painting(const window_resources& resources) noexcept
    {
        if (resources)
        {
            glViewport
            (
                0, 0,
                narrow_cast<GLsizei>(resources.sizes.width()),
                narrow_cast<GLsizei>(resources.sizes.height())
            );
        }

        return
        {
            resource_construct,
            resources.display,
            resources.surface
        };
    }

    [[nodiscard]]
    constexpr px::size2d_t sizes(const window_resources& resources) noexcept
    {
        return resources.sizes;
    }

    class window_factory
    {
    public:
        window_factory& title(std::wstring title) noexcept
        {
            app_.title(std::move(title));
            return *this;
        }

        window_factory& window_type(ui::unique_window_type_t type) noexcept
        {
            app_.type(std::move(type));
            return *this;
        }

        [[nodiscard]]
        window_t create() noexcept;

    private:
        ui::window_factory app_;
    };
}