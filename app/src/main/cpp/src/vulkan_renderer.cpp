#include "vulkan_renderer.hpp"
#include <android_native_app_glue.h>
#include <set>
#include <algorithm>

void vulkan_renderer::reset(ANativeWindow *win, AAssetManager *mgr) {

}

void vulkan_renderer::cleanup() {
    vkDestroyDevice(device,nullptr);
    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
    initialized = false;
}

void vulkan_renderer::initialize() {
    create_instance();
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_swapchain();


    initialized = true;
}

void vulkan_renderer::create_instance() {
    auto req_ext = get_required_extensions();
    VkApplicationInfo app_inf {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "VkAndroidTest",
        .applicationVersion = VK_MAKE_VERSION(1,0,0),
        .pEngineName = "Test Engine",
        .engineVersion = VK_MAKE_VERSION(1,0,0),
        .apiVersion = VK_API_VERSION_1_0
    };
    VkInstanceCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_inf,
        .enabledExtensionCount = static_cast<uint32_t>(req_ext.size()),
        .ppEnabledExtensionNames = req_ext.data()
    };
    //  uint32_t ext_count;
    VK_CHECK(vkCreateInstance(&ci,nullptr,&instance));



}

bool vulkan_renderer::is_device_usable(VkPhysicalDevice l_device) {
    VkPhysicalDeviceProperties devprops;
    VkPhysicalDeviceFeatures devfeatures;
    vkGetPhysicalDeviceProperties(l_device, &devprops);
    vkGetPhysicalDeviceFeatures(l_device, &devfeatures);

    bool extensions_suppored = check_device_extension_support(l_device);

    auto d = find_queue_families(l_device);

    bool swapchainad = false;
    if (extensions_suppored) {
        swapchain_support_details swapchain_support = query_swap_chain_support(l_device);
        swapchainad = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
    }

    return devfeatures.geometryShader && d.graphics_family.has_value() && d.present_family.has_value() && extensions_suppored && swapchainad;
}



void vulkan_renderer::pick_physical_device() {
    uint32_t devcount = 0;

    vkEnumeratePhysicalDevices(instance,&devcount,nullptr);
    if (devcount == 0) {
        throw std::runtime_error("Cannot find any gpu that supports vulkan. This device probably does NOT support vulkan.");
    }
    std::vector<VkPhysicalDevice> devices(devcount);
    vkEnumeratePhysicalDevices(instance,&devcount,devices.data());
    for (const auto& l_device : devices)
    {
        if (is_device_usable(l_device)) {
            physical_device = l_device;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) throw std::runtime_error("Unable to find a proper GPU to run this application.");



}

vulkan_renderer::queue_family_indices vulkan_renderer::find_queue_families(VkPhysicalDevice l_device) {
    queue_family_indices indices;
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(l_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queuefamilies(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(l_device, &queue_family_count, queuefamilies.data());
    for (int i = 0; i < queuefamilies.size();i++) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(l_device, i, surface, &present_support);
        if (present_support)
            indices.present_family = i;
        if (queuefamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphics_family = i;
    }
    return indices;
}

void vulkan_renderer::create_logical_device() {
    queue_family_indices indices = find_queue_families(physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_ci;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),indices.present_family.value()};
    float queue_priority = 1.0f;

    queue_ci.reserve(unique_queue_families.size());
    for (uint32_t queue_family : unique_queue_families)
        queue_ci.push_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_family,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority
        });



    VkPhysicalDeviceFeatures dev_features;

    VkDeviceCreateInfo ci{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_ci.size()),
        .pQueueCreateInfos = queue_ci.data(),
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = &dev_features
    };

    VK_CHECK(vkCreateDevice(physical_device,&ci,nullptr,&device))
    vkGetDeviceQueue(device,indices.graphics_family.value(),0,&graphics_queue);
    vkGetDeviceQueue(device,indices.present_family.value(),0,&present_queue);
}

void vulkan_renderer::create_surface() {
    VkAndroidSurfaceCreateInfoKHR ci{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .window = app->window,
    };
    VK_CHECK(vkCreateAndroidSurfaceKHR(instance,&ci,nullptr,&surface));

}

bool vulkan_renderer::check_device_extension_support(VkPhysicalDevice l_device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(l_device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(l_device, nullptr, &extension_count, available_extensions.data());
    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
    for (const auto& extension : available_extensions)
        required_extensions.erase(extension.extensionName);
    return required_extensions.empty();
}

vulkan_renderer::swapchain_support_details vulkan_renderer::query_swap_chain_support(VkPhysicalDevice l_device) {
    vulkan_renderer::swapchain_support_details details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(l_device,surface,&details.capabilities);
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(l_device,surface,&format_count,nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(l_device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(l_device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(l_device, surface, &present_mode_count, details.present_modes.data());
    }


    return details;
}

VkSurfaceFormatKHR vulkan_renderer::choose_swap_surface_format(
        const std::vector<VkSurfaceFormatKHR> &available_formats) { // Later figure out the suitable by yourself !
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }
    return available_formats[0];
}

VkExtent2D vulkan_renderer::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int32_t width = ANativeWindow_getWidth(app->window);
        int32_t height = ANativeWindow_getWidth(app->window);
        VkExtent2D actual_extent = {static_cast<uint32_t>(width),static_cast<uint32_t>(height)};
        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actual_extent;
    }
}

void vulkan_renderer::create_swapchain() {
    swapchain_support_details swapchain_support = query_swap_chain_support(physical_device);
    auto surface_format = choose_swap_surface_format(swapchain_support.formats);
    auto present_mode = choose_swap_present_mode(swapchain_support.present_modes);
    auto extent = choose_swap_extent(swapchain_support.capabilities);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR ci{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1, // This is more than 1 if its a VR app
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

}
