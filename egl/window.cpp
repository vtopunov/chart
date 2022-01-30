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

        template<size_t max_num_of_attributes>
        class attributes_builder
        {
            D_WARNING_PUSH
                D_WARNING_DISABLE_MSVC(W_unchecked_subscript_operator)
                D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized)

        public:
            static constexpr size_t size = 2_uz * max_num_of_attributes + 1_uz;

            [[nodiscard]]
            constexpr const EGLint* take() noexcept
            {
                data[position] = egl_none;
                return std::data(data);
            }

            constexpr attributes_builder& add(EGLint attribute, EGLint value) noexcept
            {
                D_ASSERT(position + 2_uz < size);
                data[position] = attribute;
                data[++position] = value;
                ++position;
                return *this;
            }

        private:
            EGLint data[size];
            size_t position{ 0_uz };

            D_WARNING_POP
        };
    }

    void window_resources_collector::operator()(const window_resources& w) const noexcept
    {
        {
            if (w.surface)
            {
                D_ASSERT_WITH_SIDE_EFFECTS(eglDestroySurface(w.display, w.surface));
            }

            if (w.context)
            {
                D_ASSERT_WITH_SIDE_EFFECTS(eglDestroyContext(w.display, w.context));
            }

            if (w.display)
            {
                eglMakeCurrent(w.display, nullptr, nullptr, nullptr);
                D_ASSERT_WITH_SIDE_EFFECTS(eglTerminate(w.display));
            }
        }

        ui::close(w.renderer_wnd);
        ui::close(w.app_wnd);
    }

    window_t window_factory::create() noexcept
    {
        window_t result;

        auto& w = as_mutable(result.r());

        w.app_wnd = app_.create().release();

        if (w.app_wnd)
        {
            w.sizes = ui::desktop_sizes();
        }

        if (w.sizes)
        {
            w.renderer_wnd
                = ui::window_factory{ app_ }
                .title({})
                .parent(w.app_wnd)
                .position(0, 0)
                .sizes(w.sizes)
                .create()
                .release();
        }

        if (w.renderer_wnd)
        {
            D_WARNING_PUSH
                D_WARNING_DISABLE_MSVC(W_do_not_use_reinterpret_cast)

                if (const auto eglGetPlatformDisplayEXT
                    = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT")))
                {
                    constexpr EGLint display_attributes[] =
                    {
                        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE,
                        egl_none
                    };

                    w.display = static_cast<display_descriptor_t>(eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, nullptr, display_attributes));
                }

            D_WARNING_POP
        }


        if (w.display)
        {
            if (!eglInitialize(w.display, nullptr, nullptr) || !eglBindAPI(EGL_OPENGL_ES_API))
            {
                result.reset();
            }
        }

        EGLConfig config{ nullptr };
        if (w.display)
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
            const auto choose_ok = eglChooseConfig(w.display, config_attributes, &config, 1, &config_count);

            if (!choose_ok || (config_count != 1))
            {
                config = nullptr;
            }
        }

        const auto extensions = query_extensions(w.display);

        if (config)
        {
            attributes_builder<2_uz> surface_attributes;

            surface_attributes.add(EGL_DIRECT_COMPOSITION_ANGLE, EGL_TRUE);

            if (extensions.has("EGL_NV_post_sub_buffer"sv))
            {
                surface_attributes.add(EGL_POST_SUB_BUFFER_SUPPORTED_NV, EGL_TRUE);
            }

            w.surface = static_cast<surface_descriptor_t>(eglCreateWindowSurface
            (
                w.display,
                config,
                w.renderer_wnd.handle,
                surface_attributes.take()
            ));
        }

        if (w.surface)
        {
            attributes_builder<5_uz> context_attributes;

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

            w.context = static_cast<context_descriptor_t>(eglCreateContext(w.display, config, nullptr, context_attributes.take()));
        }

        if (w.context)
        {
            if (!eglMakeCurrent(w.display, w.surface, w.surface, w.context))
            {
                result.reset();
            }
        }

        return result;
    }
}

