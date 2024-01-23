#include <RenderEngine/graphics/SwapChain.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/user_interface/Window.hpp>
using namespace RenderEngine;

SwapChain::SwapChain(const GPU& _gpu, const Window& window) : gpu(_gpu)
{
    if (!gpu._graphics_family_queue.has_value() || !gpu._present_family_queue.has_value())
    {
        THROW_ERROR("The provided GPU does not supports presenting to windows")
    }
    const VkSurfaceKHR& surface = window._get_vk_surface();
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu._physical_device, surface, &capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu._physical_device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu._physical_device, surface, &formatCount, formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu._physical_device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        present_modes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu._physical_device, surface, &presentModeCount, present_modes.data());
    }
    VkSurfaceFormatKHR surfaceFormat = _choose_swap_surface_format(formats);
    VkPresentModeKHR presentMode = _choose_swap_present_mode(present_modes);
    VkExtent2D extent = _choose_swap_extent(capabilities, window);
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swap_chain_infos{};
    swap_chain_infos.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_infos.surface = surface;
    swap_chain_infos.minImageCount = imageCount;
    swap_chain_infos.imageFormat = surfaceFormat.format;
    swap_chain_infos.imageColorSpace = surfaceFormat.colorSpace;
    swap_chain_infos.imageExtent = extent;
    swap_chain_infos.imageArrayLayers = 1;
    swap_chain_infos.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (gpu._graphics_family_queue.value() != gpu._present_family_queue.value())
    {
        std::vector<uint32_t> family_indices = {gpu._graphics_family_queue.value().first, gpu._present_family_queue.value().first};
        swap_chain_infos.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_infos.queueFamilyIndexCount = 2;
        swap_chain_infos.pQueueFamilyIndices = family_indices.data();
    }
    else
    {
        swap_chain_infos.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swap_chain_infos.preTransform = capabilities.currentTransform;
    swap_chain_infos.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    swap_chain_infos.presentMode = presentMode;
    swap_chain_infos.clipped = VK_TRUE;
    swap_chain_infos.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(gpu._logical_device, &swap_chain_infos, nullptr, &_swap_chain) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create the swap chain")
    }
    // Getting the vkImages
    vkGetSwapchainImagesKHR(gpu._logical_device, _swap_chain, &imageCount, nullptr);
    _vk_images.resize(imageCount);
    vkGetSwapchainImagesKHR(gpu._logical_device, _swap_chain, &imageCount, _vk_images.data());
    _image_format = surfaceFormat.format;
}

SwapChain::~SwapChain()
{
    vkDestroySwapchainKHR(gpu._logical_device, _swap_chain, nullptr);
}

VkSurfaceFormatKHR SwapChain::_choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (const auto& availableFormat : available_formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR SwapChain::_choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
    for (const auto& availablePresentMode : available_present_modes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::_choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window._get_state()->_glfw_window, &width, &height);
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}
