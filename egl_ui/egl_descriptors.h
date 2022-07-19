#pragma once

#include <egl_ui/egl_config.h>

namespace egl_ui
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
}