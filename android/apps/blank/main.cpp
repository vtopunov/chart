#include <android/sensor.h>

#include <android_native_app_glue.h>


namespace
{
    struct engine
    {
        android_app* app;

        ASensorManager* sensor_manager;
        ASensorEventQueue* sensor_event_queue;
    };

    int engine_init_display(engine* engine)
    {
        return 0;
    }


    void engine_draw_frame(engine* engine)
    {
    }


    void engine_term_display(struct engine* engine)
    {
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

    engine.sensor_manager = ASensorManager_getInstance();

    engine.sensor_event_queue = ASensorManager_createEventQueue
    (
        engine.sensor_manager,
        state->looper,
        LOOPER_ID_USER,
        nullptr,
        nullptr
    );

    {
        int events{ 0 };
        android_poll_source* source{ nullptr };

        for(;;)
        {
            if (const auto ident = ALooper_pollAll(0, nullptr, &events, (void**)&source); ident >= 0)
            {
                if (source)
                {
                    source->process(state, source);
                }

                if (state->destroyRequested)
                {
                    engine_term_display(&engine);
                    return;
                }
            }

            engine_draw_frame(&engine);
        };
    }
}
