#pragma once

#include <core/resource.h>

#include <egli/fwd.h>


namespace egli
{
    [[nodiscard]]
    error_code_t error_code() noexcept;

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

        using display_descriptor_t = egl_descriptor_t<descriptor_type_id::display, egl_display_t>;
        using surface_descriptor_t = egl_descriptor_t<descriptor_type_id::surface, egl_surface_t>;
        using context_descriptor_t = egl_descriptor_t<descriptor_type_id::context, egl_context_t>;
    }

    using private_detail_egl_descriptor::display_descriptor_t;
    using private_detail_egl_descriptor::surface_descriptor_t;
    using private_detail_egl_descriptor::context_descriptor_t;

    struct display_surface
    {
        display_descriptor_t display;
        surface_descriptor_t surface;
    };

    struct egl_context_resource
    {
        using view_type = display_surface;

        struct null_type
        {
            [[nodiscard]]
            constexpr operator egl_context_resource() const noexcept
            {
                return egl_context_resource
                {
                    .display{ nullptr },
                    .surface{ nullptr },
                    .context{ nullptr },
                };
            }
        };

        constexpr operator view_type () const noexcept
        {
            return
            {
                .display{ display },
                .surface{ surface }
            };
        }

        constexpr explicit operator bool() const noexcept
        {
            return !!context;
        }

        display_descriptor_t display;
        surface_descriptor_t surface;
        context_descriptor_t context;
    };

    static_assert(std::is_same_v<null_t<egl_context_resource>, egl_context_resource::null_type>);
    static_assert(std::is_same_v<view_t<egl_context_resource>, const egl_context_resource::view_type>);

    struct egl_context_resource_collector
    {
        void operator () (egl_context_resource r) const noexcept;
    };

    using egl_context = unique_resource<egl_context_resource, egl_context_resource_collector>;

    [[nodiscard]]
    egl_context create_egl_context(ui::window_handle_t window) noexcept;

    inline bool swap_buffers(display_surface ds) noexcept
    {
        return egl_to_bool(eglSwapBuffers(ds.display, ds.surface));
    }

    class egl_painting_owner
    {
    public:
        constexpr explicit egl_painting_owner(display_surface ds) noexcept
            : lock_{ ds }
        {}

    private:
        struct collector
        {
            void operator () (display_surface ds) const noexcept
            {
                D_ASSERT_OR_UNUSED(swap_buffers(ds));
            }
        };

        unique_resource<display_surface, collector> lock_;
    };
}

using egli::egl_context;
using egli::create_egl_context;
using egli::egl_painting_owner;