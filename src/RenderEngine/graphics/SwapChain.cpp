#include <RenderEngine/graphics/SwapChain.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/user_interface/Window.hpp>
using namespace RenderEngine;

SwapChain::SwapChain(const GPU& _gpu, const Window& window) : gpu(&_gpu)
{
    if (!gpu->_graphics_family_queue.has_value() || !gpu->_present_family_queue.has_value())
    {
        THROW_ERROR("The provided GPU does not support presenting to windows")
    }
    // Query window surface capacibilties
    const VkSurfaceKHR& surface = window._get_vk_surface();
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu->_physical_device, surface, &capabilities);
    // Choosing surface format
    uint32_t n_supported_formats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->_physical_device, surface, &n_supported_formats, nullptr);
    if (n_supported_formats == 0)
    {
        THROW_ERROR("The GPU supports 0 surface formats.")
    }
    std::vector<VkSurfaceFormatKHR> supported_formats(n_supported_formats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->_physical_device, surface, &n_supported_formats, supported_formats.data());
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
        THROW_ERROR("GPU does not support an appropriate surface format.")
    }
    // Choosing present mode
    uint32_t n_present_modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->_physical_device, surface, &n_present_modes, nullptr);
    if (n_present_modes == 0)
    {
        THROW_ERROR("The GPU supports 0 presentation modes to a screen.")
    }
    std::vector<VkPresentModeKHR> supported_present_modes(n_present_modes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->_physical_device, surface, &n_present_modes, supported_present_modes.data());
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
        glfwGetFramebufferSize(window._state->_glfw_window, &width, &height);
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
    if (gpu->_graphics_family_queue.value() != gpu->_present_family_queue.value())
    {
        std::vector<uint32_t> family_indices = {gpu->_graphics_family_queue.value().first, gpu->_present_family_queue.value().first};
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
    swap_chain_infos.presentMode = present_mode;
    swap_chain_infos.clipped = VK_TRUE;
    swap_chain_infos.oldSwapchain = VK_NULL_HANDLE;
    _swap_chain.reset(new VkSwapchainKHR, std::bind(SwapChain::_deallocate_swap_chain, std::placeholders::_1, *gpu));
    if (vkCreateSwapchainKHR(*gpu->_logical_device, &swap_chain_infos, nullptr, _swap_chain.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create the swap chain")
    }
    // Getting the vkImages
    vkGetSwapchainImagesKHR(*gpu->_logical_device, *_swap_chain, &image_count, nullptr); // just in case our requested image count was not respected, query it again
    std::vector<VkImage> vk_images(image_count);
    vkGetSwapchainImagesKHR(*gpu->_logical_device, *_swap_chain, &image_count, vk_images.data());
    images.reset(new std::vector<Image>);
    for (int i=0; i<image_count; i++)
    {
        images->push_back(Image(*gpu, extent.width, extent.height, static_cast<Image::Format>(surface_format.format), vk_images[i]));
    }
}

SwapChain::~SwapChain()
{
}

void SwapChain::operator=(const SwapChain& other)
{
    gpu = other.gpu;
    images = other.images;
    _swap_chain = other._swap_chain;
}

void SwapChain::_deallocate_swap_chain(VkSwapchainKHR* swap_chain, const GPU& gpu)
{
    vkDestroySwapchainKHR(*gpu._logical_device, *swap_chain, nullptr);
    delete swap_chain;
}
