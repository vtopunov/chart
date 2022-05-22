#include <malloc.h>

using namespace std::string_view_literals;

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "hello_triangle", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "hello_triangle", __VA_ARGS__))

namespace
{
    struct engine
    {
        struct android_app* app;

        ASensorManager* sensorManager;
        ASensorEventQueue* sensorEventQueue;

        EGLDisplay display;
        EGLSurface surface;
        EGLContext context;
        uint32_t cnt;
        int32_t width;
        int32_t height;
        GLuint program;
    };


    constexpr EGLint egl_none = EGL_NONE;

    template<size_t max_num_of_attributes>
    class egl_attributes_builder
    {
    public:
        static constexpr size_t size = 2u * max_num_of_attributes + 1u;

        const EGLint* take() noexcept
        {
            data[position] = egl_none;
            return data;
        }

        egl_attributes_builder& add(EGLint attribute, EGLint value) noexcept
        {
            assert(position + 2u < size);
            data[position] = attribute;
            data[++position] = value;
            ++position;
            return *this;
        }

    private:
        EGLint data[size];
        size_t position{ 0u };
    };

    struct egl_extensions
    {
        std::string_view extensions;

        constexpr bool has(std::string_view extention) const noexcept
        {
            return extensions.find(extention) != extensions.npos;
        }
    };

    egl_extensions query_extensions(EGLDisplay display) noexcept
    {
        const auto extensions = eglQueryString(display, EGL_EXTENSIONS);
        return
        {
            (extensions)
            ? std::string_view{extensions}
            : std::string_view{}
        };
    }

    inline void attach_shader_source(GLuint shader, std::string_view source) noexcept
    {
        const auto sourceData = source.data();
        const auto sourceLength = static_cast<GLint>(source.size());
        glShaderSource(shader, 1, &sourceData, &sourceLength);
    }

    GLuint compile_shader(GLenum type, std::string_view source)
    {
        GLuint shader = glCreateShader(type);

        attach_shader_source(shader, source);
        glCompileShader(shader);


        GLint compileResult;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

        if (compileResult == 0)
        {
            GLint infoLogLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            if (infoLogLength > 1)
            {
                std::string infoLog(static_cast<size_t>(infoLogLength), '\0');
                glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), nullptr, &infoLog[0]);
                LOGW("shader compilation failed: %s", infoLog.c_str());
            }
            else
            {
                LOGW("shader compilation failed. <Empty log message>");
            }

            glDeleteShader(shader);
            shader = 0;
        }

        return shader;
    }

    bool link_program_status(GLuint program) noexcept
    {
        GLint status = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        return !!status;
    }

    GLuint compile_program(std::string_view vsSource, std::string_view fsSource)
    {
        GLuint program = glCreateProgram();

        GLuint vs = compile_shader(GL_VERTEX_SHADER, vsSource);
        GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fsSource);

        if (vs == 0 || fs == 0)
        {
            glDeleteShader(fs);
            glDeleteShader(vs);
            glDeleteProgram(program);
            return 0;
        }

        glAttachShader(program, vs);
        glDeleteShader(vs);

        glAttachShader(program, fs);
        glDeleteShader(fs);

        glLinkProgram(program);

        if (!link_program_status(program))
        {
            glDeleteProgram(program);
            return 0;
        }

        return program;
    }

    int engine_init_display(engine* engine)
    {
        constexpr EGLint config_attribs[]
        {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
        };

        const auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        {
            EGLint major = 2, minor = 0;
            eglInitialize(display, &major, &minor);
            LOGI("EGL version %d.%d", major, minor);
        }

        EGLConfig config;

        {
            EGLint numConfigs;
            eglChooseConfig(display, config_attribs, &config, 1, &numConfigs);
        }

        EGLint format;
        eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

        ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

        const auto extensions = query_extensions(display);

        const auto surface = eglCreateWindowSurface(display, config, engine->app->window, nullptr);

        egl_attributes_builder<5u> context_attributes;

        if (extensions.has("EGL_KHR_create_context"sv))
        {
            context_attributes.add(EGL_CONTEXT_MAJOR_VERSION_KHR, 2);
            context_attributes.add(EGL_CONTEXT_MINOR_VERSION_KHR, 0);
        }

        const auto context = eglCreateContext(display, config, nullptr, context_attributes.take());

        if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
        {
            LOGW("Unable to eglMakeCurrent");
            return -1;
        }

        {
            const auto glVersion = glGetString(GL_VERSION);
            LOGI("GL version %s", glVersion);
        }

        EGLint w;
        eglQuerySurface(display, surface, EGL_WIDTH, &w);

        EGLint h;
        eglQuerySurface(display, surface, EGL_HEIGHT, &h);

        engine->display = display;
        engine->context = context;
        engine->surface = surface;
        engine->width = w;
        engine->height = h;

        engine->program = compile_program
        (
            R"(
            attribute vec2 vPosition;    
            void main()
            {
                gl_Position = vec4(vPosition, 0.0, 1.0);
            }
        )"sv,
            R"(
            precision mediump float;
            void main()
            {
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            }
        )"sv
        );

        return 0;
    }


    void engine_draw_frame(engine* engine)
    {
        if (engine->display)
        {
            const auto angle = (++(engine->cnt) & 0x7f) / 128.0f;

            glViewport(0, 0, engine->width, engine->height);

            glClearColor(angle, angle, angle, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(engine->program);

            constexpr GLfloat vertices[] =
            {
                0.0f, 0.5f,
                -0.5f, -0.5f,
                0.5f, -0.5f
            };

            constexpr size_t dim{ 2u };
            constexpr GLuint attrib_position_id{ 0u };

            glVertexAttribPointer(attrib_position_id, static_cast<GLuint>(dim), GL_FLOAT, GL_FALSE, 0, vertices);
            glEnableVertexAttribArray(attrib_position_id);

            {
                constexpr auto count = std::size(vertices) / dim;
                glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(count));
            }

            eglSwapBuffers(engine->display, engine->surface);
        }
    }


    void engine_term_display(struct engine* engine)
    {
        if (engine->display != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (engine->context != EGL_NO_CONTEXT)
            {
                eglDestroyContext(engine->display, engine->context);
            }

            if (engine->surface != EGL_NO_SURFACE)
            {
                eglDestroySurface(engine->display, engine->surface);
            }

            eglTerminate(engine->display);
        }

        engine->display = EGL_NO_DISPLAY;
        engine->context = EGL_NO_CONTEXT;
        engine->surface = EGL_NO_SURFACE;
    }

    int32_t engine_handle_input(android_app* app, AInputEvent* event)
    {
        return 0;
    }

    void engine_handle_cmd(android_app* app, int32_t cmd)
    {
        const auto data = static_cast<engine*>(app->userData);

        switch (cmd)
        {
        case APP_CMD_INIT_WINDOW:
            if (data->app->window)
            {
                engine_init_display(data);
                engine_draw_frame(data);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            engine_term_display(data);
            break;

        case APP_CMD_LOST_FOCUS:
            engine_draw_frame(data);
            break;
        }
    }
}

void android_main(android_app* state)
{
    engine engine{.app = state};

    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    engine.sensorManager = ASensorManager_getInstance();

    engine.sensorEventQueue = ASensorManager_createEventQueue
    (
        engine.sensorManager,
        state->looper,
        LOOPER_ID_USER,
        nullptr,
        nullptr
    );

    for (;;)
    {
        int events;
        android_poll_source* source;

        if (const auto ident = ALooper_pollAll(0, nullptr, &events, (void**) &source); ident >= 0)
        {
            if (source)
            {
                source->process(state, source);
            }

            if (state->destroyRequested != 0)
            {
                engine_term_display(&engine);
                return;
            }
        }

        engine_draw_frame(&engine);
    }
}
