#pragma once

#include <egl/config.h>
#include <egl/ui_wrapper.h>

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

    using private_detail_egl_descriptor::display_descriptor_t;
    using private_detail_egl_descriptor::surface_descriptor_t;
    using private_detail_egl_descriptor::context_descriptor_t;

    struct egl_resources
    {
        struct null_type
        {
            [[nodiscard]]
            constexpr operator egl_resources() const noexcept
            {
                return egl_resources
                {
                    .ui = nullui,
                    .display{ nullptr },
                    .surface{ nullptr },
                    .context{ nullptr }
                };
            }
        };

        ui_resources ui;

        display_descriptor_t display;
        surface_descriptor_t surface;
        context_descriptor_t context;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!context;
        }
    };

    [[nodiscard]]
    constexpr px::size2d sizes(const egl_resources& egl) noexcept
    {
        return egl.ui.sizes;
    }

    [[nodiscard]]
    constexpr pxside_t width(const egl_resources& egl) noexcept
    {
        return egl.ui.sizes.width();
    }

    [[nodiscard]]
    constexpr pxside_t height(const egl_resources& egl) noexcept
    {
        return egl.ui.sizes.height();
    }

#ifdef D_OS_ANDROID
    inline void quit(const egl_resources& egl) noexcept
    {
        return quit(egl.ui);
    }

#endif

    struct resources_collector
    {
        void operator () (const egl_resources& egl) const noexcept;
    };

    using egl_t = unique_resource<egl_resources, resources_collector>;

    [[nodiscard]]
    egl_t instance(os::module_handle_t module) noexcept;

#ifdef D_OS_WINDOWS
    inline egl_t instance() noexcept
    {
        return instance(nullptr);
    }

#endif

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

    class painting_owner
    {
    public:
        painting_owner(const egl_resources& egl) noexcept
            : lock_{ resource_construct, egl.display, egl.surface }
        {
            glViewport
            (
                0, 0,
                narrow_cast<GLsizei>(width(egl)),
                narrow_cast<GLsizei>(height(egl))
            );
        }

    private:
        unique_resource<display_surface, painting_collector> lock_;
    };
}

using egl::egl_resources;
using egl::egl_t;