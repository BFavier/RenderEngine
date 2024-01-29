#include <RenderEngine/graphics/SwapChain.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/user_interface/Window.hpp>
using namespace RenderEngine;

/*
#include <RenderEngine/user_interface/WindowSettings.hpp>

VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const bool enableValidationLayers = true;


struct QueueFamilyIndicesS {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

QueueFamilyIndicesS findQueueFamiliesS(const Window& window, VkPhysicalDevice device) {
    QueueFamilyIndicesS indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window._vk_surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void createLogicalDevice(const Window& window, const GPU& gpu) {
    QueueFamilyIndicesS indices = findQueueFamiliesS(window, gpu._physical_device);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    //vkGetPhysicalDeviceFeatures(gpu._physical_device, &deviceFeatures);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(gpu._physical_device, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(const Window& window, VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window._vk_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, window._vk_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, window._vk_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, window._vk_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, window._vk_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

SwapChain::SwapChain(const GPU& _gpu, const Window& window) : gpu(_gpu)
{
    device = gpu._logical_device;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(window, gpu._physical_device);
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(window._glfw_window, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = window._vk_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndicesS indices = findQueueFamiliesS(window, gpu._physical_device);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &_swap_chain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
}
*/

SwapChain::SwapChain(const GPU& _gpu, const Window& window) : gpu(_gpu)
{
    if (!gpu._graphics_family_queue.has_value() || !gpu._present_family_queue.has_value())
    {
        THROW_ERROR("The provided GPU does not support presenting to windows");
    }
    // Query window surface capacibilties
    const VkSurfaceKHR& surface = window._vk_surface;
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu._physical_device, surface, &capabilities);
    // Choosing surface format
    uint32_t n_supported_formats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu._physical_device, surface, &n_supported_formats, nullptr);
    if (n_supported_formats == 0)
    {
        THROW_ERROR("The GPU supports 0 surface formats.");
    }
    std::vector<VkSurfaceFormatKHR> supported_formats(n_supported_formats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu._physical_device, surface, &n_supported_formats, supported_formats.data());
    VkSurfaceFormatKHR surface_format;
    bool format_found = false;
    for (const VkSurfaceFormatKHR& sf : supported_formats)
    {
        if (sf.format == static_cast<VkFormat>(Image::Format::RGBA) && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format = sf;
            format_found = true;
            break;
        }
    }
    if (!format_found)
    {
        THROW_ERROR("GPU does not support an appropriate surface format.");
    }
    // Choosing present mode
    uint32_t n_present_modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu._physical_device, surface, &n_present_modes, nullptr);
    if (n_present_modes == 0)
    {
        THROW_ERROR("The GPU supports 0 presentation modes to a screen.");
    }
    std::vector<VkPresentModeKHR> supported_present_modes(n_present_modes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu._physical_device, surface, &n_present_modes, supported_present_modes.data());
    VkPresentModeKHR present_mode;
    if (!window.vsync() &&
        (std::find(supported_present_modes.begin(), supported_present_modes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != supported_present_modes.end()))
    {
        present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else if (window.vsync() &&
        (std::find(supported_present_modes.begin(), supported_present_modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != supported_present_modes.end()))
    {
        present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else if (std::find(supported_present_modes.begin(), supported_present_modes.end(), VK_PRESENT_MODE_FIFO_KHR) != supported_present_modes.end())
    {
        present_mode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else
    {
        present_mode = supported_present_modes.front();
    }
    // Choosing extent
    VkExtent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        extent = capabilities.currentExtent;
    }
    else // for Mac retina screens ... 
    {
        int width, height;
        glfwGetFramebufferSize(window._glfw_window, &width, &height);
        extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
    // create swap chain
    uint32_t image_count = std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);
    VkSwapchainCreateInfoKHR swap_chain_infos{};
    swap_chain_infos.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_infos.surface = surface;
    swap_chain_infos.minImageCount = image_count;
    swap_chain_infos.imageFormat = surface_format.format;
    swap_chain_infos.imageColorSpace = surface_format.colorSpace;
    swap_chain_infos.imageExtent = extent;
    swap_chain_infos.imageArrayLayers = 1;
    swap_chain_infos.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (gpu._graphics_family_queue.value() != gpu._present_family_queue.value())
    {
        std::vector<uint32_t> family_indices = {gpu._graphics_family_queue.value().first, gpu._present_family_queue.value().first};
        swap_chain_infos.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_infos.queueFamilyIndexCount = family_indices.size();
        swap_chain_infos.pQueueFamilyIndices = family_indices.data();
    }
    else
    {
        swap_chain_infos.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_chain_infos.queueFamilyIndexCount = 0;
        swap_chain_infos.pQueueFamilyIndices = nullptr;
    }
    swap_chain_infos.preTransform = capabilities.currentTransform;
    swap_chain_infos.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    swap_chain_infos.presentMode = present_mode;
    swap_chain_infos.clipped = VK_TRUE;
    swap_chain_infos.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(gpu._logical_device, &swap_chain_infos, nullptr, &_swap_chain) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create the swap chain");
    }
    // Getting the vkImages
    vkGetSwapchainImagesKHR(gpu._logical_device, _swap_chain, &image_count, nullptr); // just in case our requested image count was not respected, query it again
    std::vector<VkImage> vk_images(image_count, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(gpu._logical_device, _swap_chain, &image_count, vk_images.data());
    // for (int i=0; i<image_count; i++)
    // {
    //     images.emplace_back(Image(gpu, extent.width, extent.height, static_cast<Image::Format>(surface_format.format), vk_images[i]));
    // }
}

SwapChain::~SwapChain()
{
    vkDestroySwapchainKHR(gpu._logical_device, _swap_chain, nullptr);
}
