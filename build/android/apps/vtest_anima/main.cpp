#include <cassert>

#include <GLES2/gl2.h>

#include <android/native_window.h>
#include <android/sensor.h>

#include <android_native_app_glue.h>

#include <debug/debug.h>
#include <egl/window.h>

using namespace std::string_view_literals;

namespace
{
    struct engine
    {
        egl::window egl;

        uint32_t cnt;
        GLuint program;
    };

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

        if (!compileResult)
        {
            GLint infoLogLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            if (infoLogLength > 1)
            {
                std::string infoLog(static_cast<size_t>(infoLogLength), '\0');
                glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), nullptr, &infoLog[0]);
                e_debug("shader compilation failed: {}", infoLog.c_str());
            }
            else
            {
                e_debug("shader compilation failed. <Empty log message>");
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

    int engine_init_display(engine& engine)
    {
        engine.program = compile_program
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
        egl::painting_owner own{ engine->egl };

        const auto angle = (++(engine->cnt) & 0x7f) / 128.0f;

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
        case APP_CMD_TERM_WINDOW:
        case APP_CMD_PAUSE:
        case APP_CMD_STOP:
        case APP_CMD_DESTROY:
            data->egl.reset();
            break;
        }
    }
}

int app_main(os::module_handle_t app)
{
    debug("android_main");

    engine engine{ .egl{ egl::create_window(app) } };

    if (!engine.egl)
    {
        e_debug("egl error {}", eglGetError());
        return EXIT_FAILURE;
    }

    engine_init_display(engine);

    app->userData = &engine;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;

    int events{ 0 };
    android_poll_source* source{ nullptr };

    while (engine.egl && !(app->destroyRequested))
    {
        engine_draw_frame(&engine);

        if (const auto ident = ALooper_pollAll(0, nullptr, &events, (void**)&source); ident >= 0)
        {
            if (source)
            {
                source->process(app, source);
            }
        }
    }

    return EXIT_SUCCESS;
}
