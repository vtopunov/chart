#include "window.h"

using namespace std::string_view_literals;

namespace egl
{
    namespace
    {
        struct extensions
        {
            std::string_view extensions;

            constexpr bool has(std::string_view extention) const noexcept
            {
                return extensions.find(extention) != extensions.npos;
            }
        };

        [[nodiscard]]
        extensions query_extensions(EGLDisplay display) noexcept
        {
            const auto extensions = eglQueryString(display, EGL_EXTENSIONS);
            return
            {
                (extensions)
                ? std::string_view{extensions}
                : std::string_view{}
            };
        }

        constexpr EGLint egl_none = EGL_NONE;

#pragma warning(push)
#pragma warning(disable : 26446) // Prefer to use gsl::at() instead of unchecked subscript operator
#pragma warning(disable : 26482) //	Only index into arrays using constant expressions
#pragma warning(disable : 26495) // 'data' is uninitialized

        template<size_t max_num_of_attributes>
        class attributes_builder
        {
        public:
            static constexpr size_t size = 2u * max_num_of_attributes + 1u;

            [[nodiscard]]
            constexpr const EGLint* take() noexcept
            {
                data[position] = egl_none;
                return std::data(data);
            }

            constexpr attributes_builder& add(EGLint attribute, EGLint value) noexcept
            {
                D_ASSERT(position + 2u < size);
                data[position] = attribute;
                data[++position] = value;
                ++position;
                return *this;
            }

        private:
            EGLint data[size];
            size_t position{ 0u };
        };

#pragma warning(pop)
    }

    void window_resource_collector::operator()(const window_resource& window, resource_destroy_t) const noexcept
    {
        {
            const auto& egl = window.egl;

            if (egl.surface)
            {
                D_ASSERT_WITH_SIDE_EFFECTS(eglDestroySurface(egl.display, egl.surface));
            }

            if (egl.context)
            {
                D_ASSERT_WITH_SIDE_EFFECTS(eglDestroyContext(egl.display, egl.context));
            }

            if (egl.display)
            {
                eglMakeCurrent(egl.display, nullptr, nullptr, nullptr);
                D_ASSERT_WITH_SIDE_EFFECTS(eglTerminate(egl.display));
            }
        }

        ui::close(window.renderer_wnd);
        ui::close(window.app_wnd);
    }

    window window_factory::create() noexcept
    {
        window result;

        auto& p = as_mutable(result.resource());

        p.app_wnd
            = app_
            .create()
            .release();

        const auto viewport = ui::desktop_sizes();

        if (p.app_wnd && viewport)
        {
            p.renderer_wnd
                = ui::window_factory{ app_ }
                .title({})
                .parent(p.app_wnd)
                .position(0, 0)
                .sizes(viewport)
                .create()
                .release();
        }

        auto& egl = p.egl;

        if (p.renderer_wnd)
        {
#pragma warning(push)
#pragma warning(disable : 26490) // Don't use reinterpret_cast
            const auto eglGetPlatformDisplayEXT
                = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
#pragma warning(pop)

            if (eglGetPlatformDisplayEXT)
            {
                constexpr EGLint display_attributes[] =
                {
                    EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE,
                    egl_none
                };

                egl.display = static_cast<display_descriptor>(eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, nullptr, display_attributes));
            }
        }

        if (egl.display)
        {
            if (!eglInitialize(egl.display, nullptr, nullptr) || !eglBindAPI(EGL_OPENGL_ES_API))
            {
                result.reset();
            }
        }

        EGLConfig config{ nullptr };
        if (egl.display)
        {
            constexpr EGLint config_attributes[] =
            {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_STENCIL_SIZE, 8,
                egl_none
            };

            EGLint config_count{ 0 };
            const auto choose_ok = eglChooseConfig(egl.display, config_attributes, &config, 1, &config_count);

            if (!choose_ok || (config_count != 1))
            {
                config = nullptr;
            }
        }

        const auto extensions = query_extensions(egl.display);

        if (config)
        {
            attributes_builder<2u> surface_attributes;

            surface_attributes.add(EGL_DIRECT_COMPOSITION_ANGLE, EGL_TRUE);

            if (extensions.has("EGL_NV_post_sub_buffer"sv))
            {
                surface_attributes.add(EGL_POST_SUB_BUFFER_SUPPORTED_NV, EGL_TRUE);
            }

            egl.surface = static_cast<surface_descriptor>(eglCreateWindowSurface
            (
                egl.display,
                config,
                p.renderer_wnd.handle,
                surface_attributes.take()
            ));
        }

        if (egl.surface)
        {
            attributes_builder<5u> context_attributes;

            if (extensions.has("EGL_KHR_create_context"sv))
            {
                context_attributes.add(EGL_CONTEXT_MAJOR_VERSION_KHR, 2);
                context_attributes.add(EGL_CONTEXT_MINOR_VERSION_KHR, 0);
                context_attributes.add(EGL_CONTEXT_OPENGL_DEBUG, EGL_FALSE);

                if (extensions.has("EGL_ANGLE_create_context_client_arrays"sv))
                {
                    context_attributes.add(EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE, EGL_TRUE);
                }
            }

            egl.context = static_cast<context_descriptor>(eglCreateContext(egl.display, config, nullptr, context_attributes.take()));
        }

        if (egl.context)
        {
            if (eglMakeCurrent(egl.display, egl.surface, egl.surface, egl.context))
            {
                p.viewport = viewport;
            }
            else
            {
                result.reset();
            }
        }

        return result;
    }
}

