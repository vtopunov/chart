#include "egl_context.h"

#include <string_view>

#include <core/assert.h>
#include <core/utility.h>


namespace egl_ui
{
    namespace private_detail_egl_descriptor
    {
        namespace
        {
            struct config_source : egl_base_descriptor_t<EGLConfig>
            {};

            using config_descriptor_t = copy_pointer_t<EGLConfig, config_source>;
        }
    }

    namespace
    {
        using private_detail_egl_descriptor::config_descriptor_t;

        display_descriptor_t get_display() noexcept
        {
#if defined(D_OS_WINDOWS)
            constexpr EGLAttrib display_attributes[] =
            {
                EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE,
                EGL_NONE
            };

            return static_cast<display_descriptor_t>(eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE, nullptr, display_attributes));

#else
            return static_cast<display_descriptor_t>(eglGetDisplay(EGL_DEFAULT_DISPLAY));

#endif
        }

        bool egl_initialize(display_descriptor_t display) noexcept
        {
            return egl_to_bool(eglInitialize(display, nullptr, nullptr));
        }

        bool egl_bind_gles() noexcept
        {
            return egl_to_bool(eglBindAPI(EGL_OPENGL_ES_API));
        }

        bool choose_config(display_descriptor_t display, const EGLint* attribs, EGLConfig* cofigs, EGLint n_configs, EGLint* n_configs_result) noexcept
        {
            return egl_to_bool(eglChooseConfig(display, attribs, cofigs, n_configs, n_configs_result));
        }

        config_descriptor_t choose_config(display_descriptor_t display, const EGLint* attribs) noexcept
        {
            constexpr EGLint n_configs{ 1 };
            EGLConfig config{ nullptr };
            GLint n_configs_result{ 0 };
            if (choose_config(display, attribs, &config, n_configs, &n_configs_result)) [[likely]]
            {
                if (n_configs == n_configs_result) [[likely]]
                {
                    return static_cast<config_descriptor_t>(config);
                }
            }

            return nullptr;
        }

        surface_descriptor_t create_surface
        (
            display_descriptor_t display, 
            config_descriptor_t config, 
            ui::window_handle_t window, 
            const EGLint* attribs
        ) noexcept
        {
            return static_cast<surface_descriptor_t>(eglCreateWindowSurface(display, config, window, attribs));
        }

        context_descriptor_t create_context(display_descriptor_t display, config_descriptor_t config, const EGLint* attribs) noexcept
        {
            return static_cast<context_descriptor_t>(eglCreateContext(display, config, nullptr, attribs));
        }

        bool make_current(display_descriptor_t display, surface_descriptor_t draw, surface_descriptor_t read, context_descriptor_t context) noexcept
        {
            return egl_to_bool(eglMakeCurrent(display, draw, read, context));
        }

        bool destroy_current(display_descriptor_t display) noexcept
        {
            return make_current(display, nullptr, nullptr, nullptr);
        }

        bool destroy_context(display_descriptor_t display, context_descriptor_t context) noexcept
        {
            return egl_to_bool(eglDestroyContext(display, context));
        }

        bool destroy_surface(display_descriptor_t display, surface_descriptor_t surface) noexcept
        {
            return egl_to_bool(eglDestroySurface(display, surface));
        }

        bool egl_terminate(display_descriptor_t display)  noexcept
        {
            return egl_to_bool(eglTerminate(display));
        }

        struct extensions
        {
            std::string_view extensions;

            [[nodiscard]]
            constexpr bool has(std::string_view extention) const noexcept
            {
                return extensions.find(extention) != extensions.npos;
            }
        };

        [[nodiscard]]
        extensions query_extensions(display_descriptor_t display) noexcept
        {
            const auto extensions = eglQueryString(display, EGL_EXTENSIONS);
            return
            {
                (extensions)
                ? std::string_view{ extensions }
                : std::string_view{}
            };
        }


        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_unchecked_subscript_operator);
        D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized);

        template<size_t max_num_of_attributes>
        class attributes_builder
        {
            static constexpr size_t static_size{ 2u * max_num_of_attributes + 1u };

        public:
            using value_type = EGLint;

            [[nodiscard]]
            constexpr const value_type* take() noexcept
            {
                write(EGL_NONE);
                return data_;
            }

            constexpr attributes_builder& add(value_type attribute, value_type value) noexcept
            {
                write(attribute);
                write(value);
                return *this;
            }

        private:
            constexpr void write(value_type value) noexcept
            {
                D_ASSERT(position_ != std::cend(data_));
                *position_ = value; ++position_;
            }

        private:
            value_type data_[static_size];
            value_type* position_{ data_ };
        };

        D_WARNING_POP;


        void gl_enable_transparent() noexcept
        {
            glEnable(GL_BLEND);
            glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    void egl_context_resource_collector::operator()(egl_context_resource r) const noexcept
    {
        if (r.display)
        {
            destroy_current(r.display);

            if (r.context)
            {
                D_ASSERT_OR_UNUSED(destroy_context(r.display, r.context));
            }

            if (r.surface)
            {
                D_ASSERT_OR_UNUSED(destroy_surface(r.display, r.surface));
            }

            D_ASSERT_OR_UNUSED(egl_terminate(r.display));
        }
    }

    egl_context create_egl_context(ui::window_handle_t window) noexcept
    {
        using namespace std::string_view_literals;

        egl_context result{};

        auto& r = as_mutable(result.r());

        if (window) [[likely]]
        {
            r.display = get_display();
        }

        config_descriptor_t config{ nullptr };
        if (r.display && egl_initialize(r.display) && egl_bind_gles()) [[likely]]
        {
            constexpr EGLint config_attributes[] =
            {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_STENCIL_SIZE, 8,
                EGL_NONE
            };

            config = choose_config(r.display, config_attributes);
        }

        const auto extensions = query_extensions(r.display);

        if (config) [[likely]]
        {
            attributes_builder<D_CONDITIONAL_OS_WINDOWS(2u, 1u)> surface_attributes;

#if defined(D_OS_WINDOWS)
            surface_attributes.add(EGL_DIRECT_COMPOSITION_ANGLE, EGL_TRUE);
#endif

            if (extensions.has("EGL_NV_post_sub_buffer"sv))
            {
                surface_attributes.add(EGL_POST_SUB_BUFFER_SUPPORTED_NV, EGL_TRUE);
            }

            r.surface = create_surface
            (
                r.display,
                config,
                window,
                surface_attributes.take()
             );
        }

        if (r.surface) [[likely]]
        {
            attributes_builder<D_CONDITIONAL_OS_WINDOWS(3u, 2u)> context_attributes;

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

            r.context = create_context(r.display, config, context_attributes.take());
        }

        if (r.context && make_current(r.display, r.surface, r.surface, r.context)) [[likely]]
        {
            gl_enable_transparent();
        }
        else
        {
            result.reset();
        }

        return result;
    }
}
