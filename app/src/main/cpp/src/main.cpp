#include <android_native_app_glue.h>
#include <vulkan_renderer.hpp>
#include <android/log.h>

void android_main(android_app * app) {
    __android_log_print(ANDROID_LOG_INFO,"VkAndroidTest","Hello android!");
}