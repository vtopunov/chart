#include "instance.h"

#include <string_view>

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


        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_unchecked_subscript_operator);
        D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized);

        template<size_t max_num_of_attributes>
        class attributes_builder
        {
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
        };

        D_WARNING_POP;

        void gl_enable_transparent() noexcept
        {
            glEnable(GL_BLEND);
            glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    void resources_collector::operator()(const egl_resources& r) const noexcept
    {
        if (r.surface)
        {
            D_ASSERT_WITH_SIDE_EFFECTS(eglDestroySurface(r.display, r.surface));
        }

        if (r.context)
        {
            D_ASSERT_WITH_SIDE_EFFECTS(eglDestroyContext(r.display, r.context));
        }

        if (r.display)
        {
            eglMakeCurrent(r.display, nullptr, nullptr, nullptr);
            D_ASSERT_WITH_SIDE_EFFECTS(eglTerminate(r.display));
        }

        constexpr ui_resources_collector close{};
        close(r.ui);
    }

    egl_t instance(os::module_handle_t module) noexcept
    {
        egl_t result;

        auto& r = as_mutable(result.r());

        r.ui = ui_intance(module).release();

        if (r.ui)
        {
#if defined(D_OS_WINDOWS)
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_use_reinterpret_cast);

            constexpr EGLint display_attributes[] =
            {
                EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE,
                egl_none
            };

            r.display = static_cast<display_descriptor_t>(eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, nullptr, display_attributes));

            D_WARNING_POP;

#else
            r.display = static_cast<display_descriptor_t>(eglGetDisplay(EGL_DEFAULT_DISPLAY));

#endif
        }

        if (r.display)
        {
            if (!eglInitialize(r.display, nullptr, nullptr) || !eglBindAPI(EGL_OPENGL_ES_API))
            {
                result.reset();
            }
        }

        EGLConfig config{ nullptr };
        if (r.display)
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
            const auto choose_ok = eglChooseConfig(r.display, config_attributes, &config, 1, &config_count);

            if (!choose_ok || (config_count != 1))
            {
                config = nullptr;
            }
        }

        const auto extensions = query_extensions(r.display);

        if (config)
        {
            attributes_builder<D_CONDITIONAL_OS_WINDOWS(2_uz, 1_uz)> surface_attributes;

#if defined(D_OS_WINDOWS)
            surface_attributes.add(EGL_DIRECT_COMPOSITION_ANGLE, EGL_TRUE);
#endif

            if (extensions.has("EGL_NV_post_sub_buffer"sv))
            {
                surface_attributes.add(EGL_POST_SUB_BUFFER_SUPPORTED_NV, EGL_TRUE);
            }

            r.surface = static_cast<surface_descriptor_t>(eglCreateWindowSurface
            (
                r.display,
                config,
                render_window(r.ui),
                surface_attributes.take()
            ));
        }

        if (r.surface)
        {
            attributes_builder<D_CONDITIONAL_OS_WINDOWS(3_uz, 2_uz)> context_attributes;

            if (extensions.has("EGL_KHR_create_context"sv))
            {
                context_attributes.add(EGL_CONTEXT_MAJOR_VERSION_KHR, 2);
                context_attributes.add(EGL_CONTEXT_MINOR_VERSION_KHR, 0);
            }

#if defined(D_OS_WINDOWS)
            if (extensions.has("EGL_ANGLE_create_context_client_arrays"sv))
            {
                context_attributes.add(EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE, EGL_TRUE);
            }

#endif

            r.context = static_cast<context_descriptor_t>(eglCreateContext(r.display, config, nullptr, context_attributes.take()));
        }

        if (r.context)
        {
            if (!eglMakeCurrent(r.display, r.surface, r.surface, r.context))
            {
                result.reset();
            }
        }

        if (result)
        {
            gl_enable_transparent();
        }

        return result;
    }
}

