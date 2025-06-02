#ifndef ANDROIDPLAYGROUND_VULKAN_RENDERER_HPP
#define ANDROIDPLAYGROUND_VULKAN_RENDERER_HPP


struct vulkan_renderer {
    vulkan_renderer(struct android_app * app) {}
    void initialize();

    void cleanup();

    struct android_app * app;
    bool initialized;
    bool can_render;
};


#endif //ANDROIDPLAYGROUND_VULKAN_RENDERER_HPP
