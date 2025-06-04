#ifndef ANDROIDPLAYGROUND_VULKAN_RENDERER_HPP
#define ANDROIDPLAYGROUND_VULKAN_RENDERER_HPP

#include <android/native_window.h>
#include <android/asset_manager.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#include <vector>
#include <android/log.h>
#include <stdexcept>
#include <optional>

#define LOG_TAG "hellovkjni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define VK_CHECK(x)                           \
  {                                           \
    VkResult err = x;                         \
    if (err) {                                \
      throw std::runtime_error("Detected vulkan error : " + std::to_string(err));         \
    }                                         \
  }

struct vulkan_renderer {
    explicit vulkan_renderer(struct android_app * app) : initialized(false), app(app) {}
    void initialize();
    void cleanup();
    void reset(ANativeWindow * win,AAssetManager * mgr);
    bool initialized;
    vulkan_renderer (const  vulkan_renderer&)  = delete;
    vulkan_renderer & operator=(const  vulkan_renderer&)  = delete;
private:

    struct queue_family_indices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
    };

    struct swapchain_support_details {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    void create_instance();
    void pick_physical_device();
    static std::vector<const char *> get_required_extensions(){
        return std::vector<const char *>{"VK_KHR_surface","VK_KHR_android_surface"};
    }
    const std::vector<const char *> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    bool is_device_usable(VkPhysicalDevice);
    queue_family_indices find_queue_families(VkPhysicalDevice l_device);
    void create_logical_device();
    void create_surface();
    bool check_device_extension_support(VkPhysicalDevice device);
    swapchain_support_details query_swap_chain_support(VkPhysicalDevice device);
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
    void create_swapchain();


    struct android_app * app;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSurfaceKHR surface;


};


#endif //ANDROIDPLAYGROUND_VULKAN_RENDERER_HPP
