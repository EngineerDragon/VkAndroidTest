#include <android_native_app_glue.h>
#include <vulkan_renderer.hpp>
#include <android/log.h>
#include <exception>

struct engine {
    vulkan_renderer renderer;
    android_app * app;
    bool can_render;

};

static void handle_cmd(struct android_app * app,int32_t cmd) {
    auto *engine = static_cast<struct engine*>(app->userData);
    switch (cmd) {
        case APP_CMD_START:
            if (engine->app->window != nullptr) {
                engine->renderer.reset(app->window, app->activity->assetManager);
                engine->renderer.initialize();
                engine->can_render = true;
            }
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            LOGI("Called - APP_CMD_INIT_WINDOW");
            if (engine->app->window != nullptr) {
                LOGI("Setting a new surface");
                engine->renderer.reset(app->window, app->activity->assetManager);
                if (!engine->renderer.initialized) {
                    LOGI("Starting application");
                    engine->renderer.initialize();
                }
                engine->can_render = true;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine->can_render = false;
            break;
        case APP_CMD_DESTROY:
            // The window is being hidden or closed, clean it up.
            LOGI("Destroying the app");
            engine->renderer.cleanup();
        default:
            break;
    }
}

int32_t handle_input_event(android_app* app, AInputEvent* event) {
  /*  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        // Handle touch events
        int32_t action = AMotionEvent_getAction(event);
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
                // Touch started
                return 1;
            case AMOTION_EVENT_ACTION_UP:
                // Touch ended
                return 1;
            case AMOTION_EVENT_ACTION_MOVE:
                // Touch moved
                return 1;
        }
    }
    else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        // Handle key events
        int32_t keyCode = AKeyEvent_getKeyCode(event);
        int32_t action = AKeyEvent_getAction(event);

        if (action == AKEY_EVENT_ACTION_DOWN) {
            // Key pressed
            if (keyCode == AKEYCODE_BACK) {
                // Handle back button
                return 1;
            }
        }
    }
*/
    return 0;
}

void frame(struct engine * engine) {

}

void android_main(android_app * app) {
    try {
        engine core = {.renderer = vulkan_renderer(app),.app = app};
        core.app->userData = static_cast<void*>(&core);
        core.app->onAppCmd = handle_cmd;

        while (!app->destroyRequested) {
            android_poll_source * source;
            auto result = ALooper_pollOnce(core.can_render ? 0 : -1, nullptr, nullptr,
                                           (void **) &source);
            if (result == ALOOPER_POLL_ERROR) {
                LOGE("ALooper_pollOnce returned an error");
                std::abort();
            }

            if (source != nullptr) {
                source->process(app, source);
            }


            if (core.can_render) {
                frame(&core);
            }
        }

    } catch (const std::exception & e) {
        __android_log_print(ANDROID_LOG_FATAL, "VkAndroidTest", "Fatal error occured! %s",e.what());
    }
}